import path = require("path");

// TODO: this is a Windows-only tray icon. There are a number of
// cross-platform solutions but none of them are wonderful. We
// should replace this when we find a decent one.

const WindowsTrayicon = require("./win32/trayicon");
const WindowsConsole = require("./win32/console");
const open = require("open");

module.exports = class Win32Tray {
  private readonly signals = ['exit']; //, 'SIGINT', 'uncaughtException'];
  private myTrayApp: any;

  public restart(localPort: number, ngrokAddress: string) {
    if(this.myTrayApp != null) {
      this.shutdown();
    }
    return this.start(localPort, ngrokAddress);
  }

  public start(localPort: number, ngrokAddress: string) {
    let menu = [];

    menu.push({
      id: "item-id-open-localhost",
      caption: `Open http://localhost:${localPort} in browser`
    });
    if(ngrokAddress) {
      menu.push({
        id: "item-id-open-ngrok",
        caption: `Open ${ngrokAddress} in browser`
      });
    }
    menu.push({
      id: "item-id-separator-1",
      caption: "-"
    });
    menu.push({
      id: "item-id-show-console",
      caption: "Show console"
    });
    menu.push({
      id: "item-id-hide-console",
      caption: "Hide console"
    });
    menu.push({
      id: "item-id-separator-2",
      caption: "-"
    });
    menu.push({
      id: "item-id-exit",
      caption: "Exit Keyman Developer Server"
    });


    this.myTrayApp = new WindowsTrayicon({
      title: "Keyman Developer Server",
      icon: path.resolve(__dirname, "site", "favicon.ico"),
      menu: menu
    });

    this.myTrayApp.item((id: string) => {
      //console.log(`Menu id selected=${id}`);
      switch (id) {
        case 'item-id-open-localhost':
          open(`http://localhost:${localPort}`);
          break;
        case 'item-id-open-ngrok':
          open(ngrokAddress);
          break;
        case 'item-id-show-console':
          WindowsConsole.showConsole();
          break;
        case 'item-id-hide-console':
          WindowsConsole.hideConsole();
          break;
        case 'item-id-exit':
          process.exit(0);
      }
    });

    //console.log('starting tray icon');
    this.signals.forEach(signal => process.on(signal, this.shutdown.bind(this)));
  }

  public shutdown() {
    if(this.myTrayApp) {
      //console.debug('Closing down tray icon');
      this.myTrayApp.exit();
      this.myTrayApp = null;
    }
  }
}