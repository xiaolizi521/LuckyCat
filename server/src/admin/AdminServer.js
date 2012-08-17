/**
 * Admin server.
 * Responsibility:
 *  -   gm:  game master
 */

require("../system/Log");

var http = require("http")
    ,express = require("express")
    , adminServerCfg = require("../config/admin.AdminServer")
    ,sessionStore = new express.session.MemoryStore
    , sessionSecret = "LuckyCatAdmin"
    ,parseCookie = express.cookieParser(sessionSecret)
    ,app = express()
    ,server = http.createServer(app)
    ,io = require("socket.io")
    ,utils = require("util")
    ,log = new Log("AdminServer");

app.configure(function() {
    app.use(express.bodyParser());    // This cause handler' on data function doesn't be called.
    app.use(parseCookie);
    app.use(express.session({store: sessionStore, secret:sessionSecret, key:"express.sid"}));
    app.use(express.methodOverride());
    app.use(app.router);
    app.use(express.static(__dirname + "/www"));  //设定静态网页服务目录
    // app.set("views", __dirname + "/views");    // 决定不使用动态模板生成页面，使用www下的静态页面+websocket的服务模式
    // app.set("view engine", "jade");

    app.use(function(err, req, res, next) {
        // if an error occurs Connect will pass it down
        // through these "error-handling" middleware
        // allowing you to respond however you like
        log.e(err);
        res.send(500, { error: 'Sorry something bad happened!' });
    });
});

// init server, init modules here
app.initInstance = function (srvConfig, callback) {
    var cfg = require("../config/admin.AdminServer");
    var cb = function() {log.d(arguments);};
    for (var i = 0; i < arguments.length; ++i) {
        var arg = arguments[i];
        switch (typeof arg) {
            case "function":    cb = arg; break;
            case "object":      cfg = arg; break;
            default: throw new Error("Invalid argument");
        }
    }
    process.nextTick(function() {
        app.initHandlers(app);
        cb(null);
    });
    /*/ init modules
    require("./Roles").initInstance(cfg.db_roles, function(err) {
        if (! err) app.initHandlers(app);
        cb(err);
    });*/
    return this;
};

// start server, begin listening
app.start = function() {
    server.listen(require("../config/admin.AdminServer").service.port);
};

app.initHandlers = function (aExpress) {
    aExpress.post("/login", require("./handler/login"));
    aExpress.get("/js/adminServer", function(req, res, next) {
        res.setHeader("Content-Type", "text/javascript");
        res.end(utils.format("var adminServer='%s';", req.headers.host));
    });
    aExpress.get("/*", aExpress.checkHttpAuthorization);

    // init socket handlers
    var sio = io.listen(server, {log:false});
    sio.set("authorization", aExpress.checkSocketAuthorization);
    sio.sockets.on("connection", require("./socket/RoleAgent"));

    aExpress.sio = sio;
};

// 检查网页连接的授权，否则转到login.html页面
app.checkHttpAuthorization = function (req, res, next) {
    if (req.path == "/login.html") {
        next();
        return;
    }
    //log.d("checkHttpAuthorization(path=", req._parsedUrl.pathname, ",sessionID=", req.session.id, "):", req.session.token);
    //使用request.session来判断是否登录
    if( req.session.token && req.session.token!==''){
        //需要判断token是否合法，合法证明已登录

        next();
    }else{
        //读取登录页面，要求登录
        //var realpath = __dirname + '/views/' + url.parse('login.html').pathname;
        //var txt = fs.readFileSync(realpath);
        //res.end(txt);
        res.redirect("/login.html");
    }
};

// 检查socket连接是否有授权，授权数据保存在cookie中
app.checkSocketAuthorization = function (req, accept) {
    var cookies = null;
    parseCookie(req, null, function(err) {
        if (! err) {
            cookies = req.signedCookies;
        }
    });
    log.d("checkSocketAuthorization(referer=", req.headers.referer, ",cookies=", cookies, ")");

    // check if there's a cookie header
    if (cookies && cookies['express.sid']) {
        // 使用sid从sessionStore中获取session
        sessionStore.load(cookies['express.sid'], function (err, session) {
            // 如果获取失败，取消连接
            if (err) {
                accept(err.message, false);
            } else {
                // 把session挂到req上，以便在下面的事件中访问
                req.session = session;
                accept(null, true);
            }
        });
    } else if (req.query.server_secret && (req.query.server_secret == adminServerCfg.server_secret)) {
        // 从别的服务器连接过来的
        accept(null, true);
    } else {
        // 连接失败
        accept(null, false);
    }
};

module.exports = app;