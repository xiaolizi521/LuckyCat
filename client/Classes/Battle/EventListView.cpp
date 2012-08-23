//
//  EventListView.cpp
//  HelloWorld
//
//  Created by XiaoZhong Zhu on 12-8-8.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "EventListView.h"
#include "BattleDefine.h"
#include "LevelDataManager.h"
#include "DictDataManager.h"
#include "EventDataManager.h"
#include "NPCDialogView.h"
#include "GeneralBattleView.h"
#include "SpecialBattleView.h"
#include "OpenBoxView.h"
#include "ChapterScene.h"
#include "HeroHeadView.h"

#include "NetManager.h"
#include "CCMessageQueue.h"
#include "json.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace std;

static bool m_bIsInEvent = false;

bool EventListView::getIsInEvent()
{
    return m_bIsInEvent;
}

void EventListView::onEnter()
{
    CCLayer::onEnter();
}

void EventListView::initLayer(const int tChapterId,const stPage *p_page, CCObject *target, SEL_CallFuncND pfnSelector)
{
    mLEventType = kLEventTypeOneEventWasFinished;
    
    m_target = target;
    m_pfnSelector = pfnSelector;
    
    p_pPage = p_page;
    
    mChapterId = tChapterId;
    
    p_HeroHeadView = NULL;
    
    if ( p_pPage == NULL )return;
    
    this->getEventDataList();
    
    this->showHeroHeadView();
};

void EventListView::getEventDataList()
{
    this->netCallBackEventList(NULL,NULL);
    NetManager::shareNetManager()->sendEx(kModeEvent, kDoGetEventList, callfuncND_selector(EventListView::netCallBackEventList), this, "\"chapterId\": %d, \"pageId\": %d, \"eventId\": %d", mChapterId, p_pPage->id, p_pPage->eventId);
}

void EventListView::netCallBackEventList(CCNode* pNode, void* data)
{
    if ( data )
    {
        ccnetwork::RequestInfo *tempInfo = static_cast<ccnetwork::RequestInfo *>(data);
        
        vector<stGood> tGoodsList;
        
        Json::Reader reader;
        Json::Value json_root;
        if (!reader.parse(tempInfo->strResponseData.c_str(), json_root))
            return;
        //
        Json::Value json_meta = json_root["meta"];
        Json::Value json_out = json_meta["out"];
        
        int ret = json_out["result"].asInt();
        if ( ret != 0 )
        {
            return;
        }
        Json::Value json_eventArray = json_out["eventList"];
        
        mEventDataList.clear();
        
        for (int i = 0; i < json_eventArray.size(); i++) {
            Json::Value jEvent = json_eventArray[i];
            LEventData *tEventData = new LEventData();
            
            tEventData->id = jEvent["id"].asInt();
            
            Json::Value jTargetArray = jEvent["targetArray"];
            std::vector<int> tVectorTarget;
            for (int j = 0; j < jTargetArray.size(); j++) {
                int tTarget = jTargetArray[j].asInt();
                tVectorTarget.push_back(tTarget);
            }
            tEventData->targetId = tVectorTarget;
            
            Json::Value jBonusArray = jEvent["bonusArray"];
            std::vector<stGood> tVectorGood;
            for (int j = 0; j < jBonusArray.size(); j++) {
                Json::Value jBonus = jBonusArray[j];
                stGood tGoods;
                tGoods.id = jBonus["id"].asInt();
                tGoods.type = jBonus["type"].asInt();
                tGoods.num = jBonus["num"].asInt();
                
                tVectorGood.push_back(tGoods);
            }
            tEventData->bonus = tVectorGood;
            
            stEvent *tStEvent = EventDataManager::getShareInstance()->getEvent(tEventData->id);
            tEventData->type = (LEventType)(jEvent["type"].asInt());
            tEventData->pStEvent = tStEvent;
            
            tEventData->pBattle = NULL;
            tEventData->box_id = jEvent["box_id"].asInt();
            
            if (tVectorGood.empty() == false) {
                tEventData->m_bBattleResultIsShowed = false;
            }
            else {
                tEventData->m_bBattleResultIsShowed = true;
            }
            mEventDataList.push_back(tEventData);
        }
        
        p_CurEvent = getCurEvent();
        
        this->showCurEvent();
    }
}

LEventData * EventListView::getCurEvent()
{
    if ( mEventDataList.empty() == false )
    {
        p_CurEvent = mEventDataList[0];
        
        switch (p_CurEvent->type) {
            case kLEventTypeGeneralBattle:
            {
                mLEventType = kLEventTypeGeneralBattle;

                break;
            }
            case kLEventTypeDialogue:
            {
                mLEventType = kLEventTypeDialogue;
                break;
            }
            case kLEventTypeSpecialBattle:
            {
                mLEventType = kLEventTypeSpecialBattle;
            }
            default:
                break;
        }
    }
    else {
        p_CurEvent = NULL;
    }
    
    return p_CurEvent;
}

void EventListView::popEvent()
{
    if ( mEventDataList.empty() == false )
    {
        std::vector<LEventData *>::iterator _iter = mEventDataList.begin();
        mEventDataList.erase(_iter);
        
        if ( mEventDataList.empty() == false )
        {
            p_CurEvent = mEventDataList[0];
            
            switch (p_CurEvent->type) {
                case kLEventTypeGeneralBattle:
                {
                    mLEventType = kLEventTypeGeneralBattle;
                    break;
                }
                case kLEventTypeDialogue:
                {
                    mLEventType = kLEventTypeDialogue;
                    break;
                }
                case kLEventTypeSpecialBattle:
                {
                    mLEventType = kLEventTypeSpecialBattle;
                    break;
                }
                default:
                {
                    mLEventType = kLEventTypeFinishedEvent;
                    p_CurEvent = NULL;
                    break;
                }
            }
        }
        else {
            mLEventType = kLEventTypeFinishedEvent;
            p_CurEvent = NULL;
        }
    }
    else {
        mLEventType = kLEventTypeFinishedEvent;
        p_CurEvent = NULL;
    }
}

LEventType EventListView::getLEventType()
{
    return mLEventType;
}

void EventListView::removeAndCleanSelf(float dt)
{
    m_bIsInEvent = false;
    
    this->removeAllChildLayer();
    
    ((m_target)->*(m_pfnSelector))(this, NULL);
}

void EventListView::callbackEventWasFinished(CCNode* node, void* data)
{
    this->scheduleOnce(schedule_selector(EventListView::showNextEvent), 0.1f);
}

void EventListView::showCurEvent()
{
    if ( mLEventType == kLEventTypeDialogue )
    {
        showDialogView();
    }
    else if ( mLEventType == kLEventTypeSpecialBattle )
    {
        showSpecialBattleView();
    }
    else if ( mLEventType == kLEventTypeGeneralBattle ){
        showGeneralBattleView();
    }
    else  if ( mLEventType == kLEventTypeFinishedEvent ||  mLEventType == kLEventTypeOneEventWasFinished ) {
        this->scheduleOnce(schedule_selector(EventListView::removeAndCleanSelf), 0.05f);
    }
}

void EventListView::showDialogView()
{
    NPCDialogView *tDialog = NPCDialogView::create(this);
    if (tDialog)
    {
        this->removeAllChildLayer();
        
        tDialog->setData(p_CurEvent, this, callfuncND_selector(EventListView::callbackEventWasFinished));
        
        p_CurLayer = static_cast<cocos2d::CCLayer *>(tDialog);
        
        this->addChild(p_CurLayer,kTagLayerDialog);
    }
}

void EventListView::showGeneralBattleView()
{
    GeneralBattleView *tGeneralBattle = GeneralBattleView::create(this);
    if (tGeneralBattle)
    {
        this->removeAllChildLayer();
        
        tGeneralBattle->setData(p_CurEvent, this, callfuncND_selector(EventListView::callbackEventWasFinished));
        
        p_CurLayer = static_cast<cocos2d::CCLayer *>(tGeneralBattle);
        
        this->addChild(p_CurLayer,kTagLayerBattle);
    }
}

void EventListView::showSpecialBattleView()
{
    SpecialBattleView *tSpecialBattle = SpecialBattleView::create(this);
    if (tSpecialBattle)
    {
        this->removeAllChildLayer();
        
        tSpecialBattle->setData(p_CurEvent, this, callfuncND_selector(EventListView::callbackEventWasFinished));
        
        p_CurLayer = static_cast<cocos2d::CCLayer *>(tSpecialBattle);
        
        this->addChild(p_CurLayer,kTagLayerBattle);
    }
}

void EventListView::showOpenBoxView()
{
    if ( p_CurEvent && p_CurEvent->m_bBoxIsOpened == false && p_CurEvent->box_id != -1 )
    {
        this->removeAllChildLayer();
        
        OpenBoxView *pLayer = OpenBoxView::create(this);
        pLayer->setSelector(this, callfuncND_selector(EventListView::callbackEventWasFinished));
        pLayer->setEvent(p_CurEvent);
        
        p_CurLayer = static_cast<cocos2d::CCLayer *>(pLayer);
        this->addChild(p_CurLayer,kTagLayerOpenBox);
    }
}

void EventListView::showHeroHeadView()
{
    if (p_HeroHeadView)
    {
        p_HeroHeadView->removeFromParentAndCleanup(true);
        p_HeroHeadView = NULL;
    }
    else {
        p_HeroHeadView = HeroHeadView::create(this);
        this->addChild(p_HeroHeadView,kTagLayerHeroHead);
    }
}

void EventListView::showBattleResultView()
{
    this->removeAllChildLayer();
    
    BattleResultView *pLayer = BattleResultView::create(this);
    pLayer->setSelector(this, callfuncND_selector(EventListView::callbackEventWasFinished));
    p_CurEvent->setBattleResultIsShowed();
    pLayer->initView(p_CurEvent);
    
    p_CurLayer = static_cast<cocos2d::CCLayer *>(pLayer);
    
    this->addChild(p_CurLayer,kTagLayerBattleResult);
}

void EventListView::showNextEvent(float dt)
{
    if ( p_CurEvent && p_CurEvent->getBattleResultIsShowed() == false )
    {
        this->showBattleResultView(); 
    }
    else if ( p_CurEvent && p_CurEvent->m_bBoxIsOpened == false && p_CurEvent->box_id != -1 ) {
        this->showOpenBoxView();
    }
    else {
        popEvent();
        
        showCurEvent();
    }
}

void EventListView::removeAllChildLayer()
{
    if ( p_CurLayer )
    {
        p_CurLayer->removeFromParentAndCleanup(true);
        
        p_CurLayer = NULL;
    }
//    if (p_HeroHeadView)
//    {
//        p_HeroHeadView->removeFromParentAndCleanup(true);
//        p_HeroHeadView = NULL;
//    }
//    if (p_OpenBoxView)
//    {
//        p_OpenBoxView->removeFromParentAndCleanup(true);
//        p_OpenBoxView = NULL;
//    }
//    if (p_BattleResultView)
//    {
//        p_BattleResultView->removeFromParentAndCleanup(true);
//        p_BattleResultView = NULL;
//    }
}