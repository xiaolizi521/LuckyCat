/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2008-2010 Ricardo Quesada
 Copyright (c) 2011      Zynga Inc.

 http://www.cocos2d-x.org


 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
(function () {
    var d = document;
    var c = {
        COCOS2D_DEBUG:2, //0 to turn debug off, 1 for basic debug, and 2 for full debug
        box2d:false,
        showFPS:true,
        frameRate:60,
        loadExtension:true,
        tag:'gameCanvas', //the dom element to run cocos2d on
        engineDir:'../libs/cocos2d-html5/cocos2d/',
        //SingleEngineFile:'',
        appFiles:[
            '../Classes/resource.js',
            '../Classes/myApp.js',

            "../Classes/Basic/Basic.js",
            "../Classes/Basic/NetBasic.js",
            "../Classes/Basic/NetManager.js",
            "../Classes/Basic/CustomXMLHTTPRequest.js",
            "../Classes/Basic/DictDataManager.js",
            "../Classes/Basic/LevelDataManager.js",

            "../Classes/Event/EventDataManager.js",
            "../Classes/Event/TouchLayer.js",
            "../Classes/Event/EventListLayer.js",
            "../Classes/Event/HeroHeadLayer.js",
            "../Classes/Event/NPCDialogLayer.js",
            "../Classes/Event/OpenBoxResultLayer.js",
            "../Classes/Event/OpenBoxLayer.js",
            "../Classes/Battle/SpecialBattleLayer.js",
            "../Classes/Battle/BattleResultLayer.js",
            "../Classes/Battle/GeneralBattleLayer.js",

            "../Classes/Main/CoverScene.js",
            "../Classes/Chapter/ChapterScene.js",
            "../Classes/Page/PageMapScene.js",
            "../Classes/Page/PageCellLayer.js",
            "../Classes/Page/PageScene.js",

            "../Classes/PlayerInfo/BasicInfoLayer.js"

            //add your own files in order here
        ]
    };
    window.addEventListener('DOMContentLoaded', function () {
        //first load engine file if specified
        var s = d.createElement('script');
        /*********Delete this section if you have packed all files into one*******/
        if (c.SingleEngineFile && !c.engineDir) {
            s.src = c.SingleEngineFile;
        }
        else if (c.engineDir && !c.SingleEngineFile) {
            s.src = c.engineDir + 'platform/jsloader.js';
        }
        else {
            alert('You must specify either the single engine file OR the engine directory in "cocos2d.js"');
        }
        /*********Delete this section if you have packed all files into one*******/

            //s.src = 'Packed_Release_File.js'; //IMPORTANT: Un-comment this line if you have packed all files into one

        d.body.appendChild(s);
        s.c = c;
        s.id = 'cocos2d-html5';
        //else if single file specified, load singlefile
    });
})();
