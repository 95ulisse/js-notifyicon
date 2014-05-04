"use strict";

var native = require('./build/Release/NotifyIcon'),
    util = require('util'),
    EventEmitter = require('events').EventEmitter,
    globalCache = {};

// NotifyIcon constructor
function NotifyIcon(opts)
{
    if (!(this instanceof NotifyIcon))
        return new NotifyIcon();

    EventEmitter.call(this);

    // Stores the native ID
    var id = native.createNotifyIcon();
    globalCache[id] = this;
    Object.defineProperty(this, "_id", {
        value: id,
        configurable: false,
        enumerable: false,
        writable: false
    });

    // Shortcuts
    if (opts) {
        opts.tooltip && this.setTooltip(opts.tooltip);
        opts.icon && this.setIcon(opts.icon);
        opts.menu && this.setMenu(opts.menu);
    }
}
util.inherits(NotifyIcon, EventEmitter);

// The other instance methods call the native ones
NotifyIcon.prototype.setTooltip = function (text) {
    native.setTooltip(this._id, text);
};
NotifyIcon.prototype.setIcon = function(icon) {
    if (!(icon instanceof native.IconHandle))
        throw new Error("Expected IconHandle. Please, load an icon using 'loadIcon'.");
    native.setIcon(this._id, icon);
};
NotifyIcon.prototype.setMenu = function (menu) {
    native.setMenu(this._id, menu);
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

    // The icon must be visible to show a message
    this.show();
    var id = this._id;
    setTimeout(function () {
        native.showMessage(id, title, text, icon || 0);
    }, 0);
};
NotifyIcon.prototype.dispose = function () {
    native.dispose(this._id);
};

// Redirects the events fired from the native extension to the correct JS instance
native.setCallback(function (id, msg) {
    globalCache[id].emit(msg);
});

// Exports on the NotifyIcon constructor the `loadIcon` function
NotifyIcon.loadIcon = native.loadIcon;

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