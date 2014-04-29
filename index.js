"use strict";

var native = require('./build/debug/NotifyIcon'),
    util = require('util'),
    EventEmitter = require('events').EventEmitter,
    globalCache = {};

// NotifyIcon constructor
function NotifyIcon()
{
    if (!(this instanceof NotifyIcon))
        return new NotifyIcon();

    EventEmitter.call(this);

    var id = native.createNotifyIcon();
    globalCache[id] = this;
    Object.defineProperty(this, "_id", {
        value: id,
        configurable: false,
        enumerable: false,
        writable: false
    });
}
util.inherits(NotifyIcon, EventEmitter);

// The other instance methods call the native ones
NotifyIcon.prototype.setTooltip = function (text) {
    native.setTooltip(this._id, text);
};
NotifyIcon.prototype.show = function () {
    native.show(this._id);
};
NotifyIcon.prototype.hide = function () {
    native.hide(this._id);
};
NotifyIcon.prototype.showMessage = function (title, text, icon) {
    icon = ({
        "none": 0,
        "info": 1,
        "warn": 2,
        "warning": 2,
        "error": 3,
        "info-big": 33,
        "warn-big": 34,
        "warning-big": 34,
        "error-big": 35
    })[ icon || "none" ];
    native.showMessage(this._id, title, text, icon || 0);
};

// Redirects the events fired from the native extension to the correct JS instance
native.setCallback(function (id, msg) {
    globalCache[id].emit(msg);
});

// Registers some cleanup code on process exit
function exitHandler(options, err) {
    if (options.cleanup)
        native.dispose();
    if (err)
        console.error(err.stack);
    if (options.exit)
        process.exit();
}
process.on('exit', exitHandler.bind(null, {cleanup:true}));
process.on('SIGINT uncaughtException', exitHandler.bind(null, {exit:true}));

// Exports the NotifyIcon constructor
module.exports = NotifyIcon;