//
//  OpenBoxView.cpp
//  LuckyCat
//
//  Created by XiaoZhong Zhu on 12-8-10.
//  Copyright (c) 2012年 厦门雅基软件有限公司. All rights reserved.
//

#include "OpenBoxView.h"
#include "extensions/CCBReader/CCBReader.h"
#include "extensions/CCBReader/CCNodeLoaderLibrary.h"
#include "EventDataManager.h"
#include "EventBasic.h"
#include "LuckySprite.h"
#include "OpenBoxResultView.h"
#include "NetManager.h"
#include "CCMessageQueue.h"
#include "json.h"

#include "FuzzyBgView.h"

#define TAG_BUTTON_OPEN_BOX 11

USING_NS_CC;
USING_NS_CC_EXT;
using namespace std;

OpenBoxView::OpenBoxView():m_bIsOpen(false)
{
    
}

OpenBoxView::~OpenBoxView()
{
    
}

OpenBoxView *OpenBoxView::create(cocos2d::CCObject * pOwner)
{
    cocos2d::extension::CCNodeLoaderLibrary * ccNodeLoaderLibrary = cocos2d::extension::CCNodeLoaderLibrary::newDefaultCCNodeLoaderLibrary();
    
    ccNodeLoaderLibrary->registerCCNodeLoader("FuzzyBgView", FuzzyBgViewLoader::loader());
    ccNodeLoaderLibrary->registerCCNodeLoader("OpenBoxView", OpenBoxViewLoader::loader());
    
    cocos2d::extension::CCBReader * ccbReader = new cocos2d::extension::CCBReader(ccNodeLoaderLibrary);
    ccbReader->autorelease();
    
    CCNode * pNode = ccbReader->readNodeGraphFromFile("pub/", "ccb/openbox.ccbi", pOwner);
    
    OpenBoxView *pOpenBoxView = static_cast<OpenBoxView *>(pNode);
    return pOpenBoxView;
}

cocos2d::SEL_MenuHandler OpenBoxView::onResolveCCBCCMenuItemSelector(cocos2d::CCObject * pTarget, cocos2d::CCString * pSelectorName)
{
    //CCB_SELECTORRESOLVER_CCMENUITEM_GLUE(this, "onMenuItemClicked", OpenBoxView::onMenuItemClicked);
    
    return NULL;
}
cocos2d::extension::SEL_CCControlHandler OpenBoxView::onResolveCCBCCControlSelector(cocos2d::CCObject * pTarget, cocos2d::CCString * pSelectorName)
{
    CCB_SELECTORRESOLVER_CCCONTROL_GLUE(this, "onCCControlButtonClicked", OpenBoxView::onCCControlButtonClicked );
    return NULL;
}
bool OpenBoxView::onAssignCCBMemberVariable(cocos2d::CCObject * pTarget, cocos2d::CCString * pMemberVariableName, cocos2d::CCNode * pNode)
{
    return false;
}

void OpenBoxView::onMenuItemClicked(cocos2d::CCObject *pTarget)
{
    
}

void OpenBoxView::onCCControlButtonClicked(cocos2d::CCObject *pSender, cocos2d::extension::CCControlEvent pCCControlEvent) {
    if ( m_bIsOpen == false && p_CurEvent && p_CurEvent->box_id != -1)
    {
        m_bIsOpen = true;
        
        char strChar[100];
        sprintf(strChar,"\"EventId\": %d,\"boxId\": %d" ,p_CurEvent->id, p_CurEvent->box_id);
        NetManager::shareNetManager()->send(kModeBox, kDoOpenBox,                                      callfuncND_selector(OpenBoxView::netCallBack), this, strChar);
    }
}

void OpenBoxView::setEvent(stEvent *t)
{
    p_CurEvent = t;
}

void OpenBoxView::netCallBack(CCNode* pNode, void* data)
{    
    if ( data )
    {
        if ( pNode->getTag() == TAG_BUTTON_OPEN_BOX )
        {
            CCControlButton *pButton = (CCControlButton *)pNode;
            pButton->cocos2d::extension::CCControl::setSelected(true);
        }
        ccnetwork::RequestInfo *tempInfo = static_cast<ccnetwork::RequestInfo *>(data);
        
        CCLog("--------> 测试返回 url : %s\n>> ReuestInfo: %s,\n>> ResponseData %s",tempInfo->strUrl.c_str(),tempInfo->strRequestData.c_str(),tempInfo->strResponseData.c_str());
        
        if (tempInfo->stateCode == CURLE_COULDNT_CONNECT )
        {
            CCLog("----> This Requst Was Time Out CURLE_COULDNT_CONNECT...");
        }
        if ( tempInfo->stateCode == CURLE_OPERATION_TIMEDOUT )
        {
            CCLog("----> This Requst Was Time Out CURLE_OPERATION_TIMEDOUT...");
        }
        
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
            //open box failed. the box id is not exit. alert player.
            
            return;
        }
        Json::Value json_goodsArray = json_out["goodsArray"];
        
        for (int i = 0; i < json_goodsArray.size(); i++) {
            Json::Value goods = json_goodsArray[i];
            stGood tmpGoods;
            tmpGoods.id = goods["id"].asInt();
            tmpGoods.type = goods["type"].asInt();
            tmpGoods.num = goods["num"].asInt();
            
            tGoodsList.push_back(tmpGoods);
        }
        
        OpenBoxResultView *pOpenBoxResult = OpenBoxResultView::create(this);
        pOpenBoxResult->initView(tGoodsList);
        this->addChild(pOpenBoxResult,99);
    }
}