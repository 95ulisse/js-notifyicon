# JS-NotifyIcon

`js-notifyicon` is a wrapper around the native Windows NotifyIcon API, which allows to display some icons in the tray.

**This project was born as an experiment to learn C++, so consider that there might be bugs, leaks and so on.**



## How to install

Just issue from the command line:

    npm install js-notifyicon

Note that since this project contains a native addon, you will need to satisfy other dependencies in order to compile the C++ source files:

* Microsoft Visual Studio C++ 2010 (at least) (express edition is fine)
* Python (v2.7.3 recommended, v3.x.x is not supported)
* `node-gyp` globally installed

With the newer versions of Visual Studio, `node-gyp` fails to recognize the correct version of the tools, so try this, if the above does not work:
    
    npm install js-taskdialog --msvs_version=2012   # 2012 if you have VS2012

If you want to know more about `node-gyp` visit its [repo](https://github.com/TooTallNate/node-gyp)!



## Getting started

First of all, call `require('js-notifyicon')` to get the `NotifyIcon` constructor. Each icon is encapsulated in an instance of this class, and can be managed separately. So, let's start by creating a new icon:

```js
var NotifyIcon = require('js-notifyicon');
var ni = new NotifyIcon({
    tooltip: 'Look ma, an icon!',
    icon: NotifyIcon.loadIcon('C:\\Windows\\System32\\shell32.dll', 13)
})
ni.show();

// Later on...
ni.hide();

// Even later, we we are SURE that the icon is not neede anymore
ni.dipose();
```

The options passed to the `NotifyIcon` constructor are pretty self-explaining. Note that to load an icon you need to call the `NotifyIcon.loadIcon` function, passing one or two parameters: if you just pass a path, it will load the icon it points to, but you can even extract icons from exe or dll files passing the path to the file and the icon index (as shown in the example). When you have setup your icon, call the `show` method to make it visible; to hide it, call `hide`, but when you want to definitely get rid of the icon, remember to call `dispose`, since this is a native object, and has resources to free!

**Note that passing parameters to the constructor is just a shortcut for calling `set*` methods.**



## Messages

A notify icon has the ability to display the user balloon notifications, and this API is exposed to you through the `showMessage` method:

```js
ni = new NotifyIcon({ ... });
ni.showMessage('Title', 'Look ma, a notification!', 'info-big');
```

`showMessage` parameters are (in order) title, contents and icon of the notification. All the available icons are:

* `none`
* `info`
* `warn`
* `warning`
* `error`
* `info-big`
* `warn-big`
* `warning-big`
* `error-big`

**Note that if the icon is not visible, it will be shown automatically when you call `showMessage`.**



## Context menu

Another useful feature for a notify icon is the context menu. To define a context menu, simply pass an array describing the menu to the `NotifyIcon` constructor.

```js
ni = new NotifyIcon({
    ...,
    menu: [
        { text: 'Command 1', command: 'cmd1' },
        { text: 'Command 2', command: 'cmd2' },
        { separator: true },
        { text: 'Command 3', command: 'cmd3' }
    ]
})

ni.on('command:cmd1', function () {
    ni.showMessage('Command clicked', 'You clicked command 1', 'info-big');
});

ni.on('command:cmd2', function () {
    ni.showMessage('Command clicked', 'You clicked command 2', 'info-big');
});

ni.on('command:cmd3', function () {
    ni.showMessage('Command clicked', 'You clicked command 3', 'info-big');
});
```

Each entry in the menu array represents a menu item, which has a `text` and a `command` property. The `command` property is just an identifier the will be used later to handle the event raised when the user selects a menu item.


# License

This project is released under the terms of the [MIT license](http://opensource.org/licenses/MIT).