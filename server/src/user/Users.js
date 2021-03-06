/**
 * user controller for create、update、delete、etc...
 *
 * sample code:
     var users = require("./user/Users");
     var info = {};
     var uuid;
     info.udid = "1234567890";
     users.createUser(info, function(uuid1) {
         console.log("createUser:"+uuid1);
         uuid = uuid1;
         users.findUser(info, function(uuid2) {
            console.log("findUser:"+uuid2);
         })
     });
 */

if (global.Users) {
    module.exports = Users;
    return;
}

require("../system/DBAgent");
require("../system/Log");

// user database connection
var log = new Log("Users")
    , util = require("util");

Users = {
    _dbAgent : null,
    initInstance : function(dbConfig, callback) {
        Users._dbAgent = new DBAgent(dbConfig);
        Users._dbAgent.connect();

        Users._dbAgent.createDatabase(dbConfig.recreate || false, function(err) {
            if (err) {
                throw err;
                return;
            }
            Users._dbAgent.query("CREATE TABLE IF NOT EXISTS users (" +
                "uuid       bigint(8)       PRIMARY KEY AUTO_INCREMENT COMMENT '自增长ID'," +
                "udid       varchar(127)," +
                "91id       varchar(127)" +
                ") DEFAULT CHARSET=utf8 COMMENT='用户表' AUTO_INCREMENT=10000",
                function (err) {
                    callback(err);
                });
        });
    },

    // uuid returned by cb
    createUser : function(info, cb) {
        var udid = info["udid"];
        if ((typeof udid == "string") && udid.length > 0) {
            Users._dbAgent.query(util.format("INSERT INTO `users` (udid) VALUES ('%s')", udid), function(err, ret) {
                if (err) {
                    throw (err);
                } else {
                    Users._dbAgent.query("SELECT LAST_INSERT_ID()", function(err, ret) {
                        if (err) {
                            throw err;
                        } else {
                            cb(ret.insertId);
                        }
                    });
                }
            });
        } else {
            throw new Error("Invalid arguments");
        }
    },

    // belong to uuid map to other id designed as 1 to 1
    // uuid returned by callback
    findUser : function(info, cb) {
        var sqlCmd = "SELECT `uuid` FROM `users` WHERE `%s`='%s'";
        var colKey, colVal;
        if ((typeof info["udid"] == "string") && info["udid"].length > 0) {
            // find user by udid
            colKey = "udid";
            colVal = info["udid"];
        }
        Users._dbAgent.query(util.format(sqlCmd, colKey, colVal), function(err, rows) {
            if (err) {
                throw err;
            } else {
                switch (rows.length) {
                    case 0: cb(null); break;
                    case 1: cb(rows[0]["uuid"]); break;
                    default: throw new Error(util.format("find user by(%s=%s) in database more than one!", colKey, colVal)); break;
                }
            }
        });
    }
};

module.exports = Users;