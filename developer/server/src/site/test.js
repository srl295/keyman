const ta1 = document.getElementById('ta1');

const devices = {
  Windows:         { name: 'Windows',           browser: 'chrome', formFactor: 'desktop', OS: 'windows', touchable: false, dimensions: [640, 300] },
  macOS:           { name: 'macOS',             browser: 'chrome', formFactor: 'desktop', OS: 'macosx',  touchable: false, dimensions: [640, 300] },
  Linux:           { name: 'Linux',             browser: 'chrome', formFactor: 'desktop', OS: 'linux',   touchable: false, dimensions: [640, 300] },
  iPhone:          { name: 'iPhone',            browser: 'chrome', formFactor: 'phone',   OS: 'ios',     touchable: true,  dimensions: [527, 280] },
  iPadMini:        { name: 'iPad Mini',         browser: 'chrome', formFactor: 'tablet',  OS: 'ios',     touchable: true,  dimensions: [829, 300] },
  Pixel5:          { name: 'Google Pixel 5',    browser: 'chrome', formFactor: 'phone',   OS: 'android', touchable: true,  dimensions: [551, 290] },
  Nexus9:          { name: 'Google Nexus 9',    browser: 'chrome', formFactor: 'tablet',  OS: 'android', touchable: true,  dimensions: [763, 300] },
};

// these need to be defined before we load inc/keyboards.js
var debugKeyboards = [];
let registeredModels = {};

keyman.addEventListener('keyboardregistered', function(keyboardProperties) {
  //console.log('keyboardregistered:'+JSON.stringify(keyboardProperties)+' [active='+keyman.getActiveKeyboard()+';'+keyman.core.activeKeyboard+']');
  buildKeyboardList();
});

keyman.addEventListener('keyboardloaded', function(keyboardProperties) {
  // Note: see #5730 for why we have a timeout here
  window.setTimeout(buildKeyboardList, 10);
});

keyman.init({
  ui:'button',
  resources:'/resource/',
  keyboards:'/data/keyboard/',
  fonts:'/data/font/',
  attachType:'auto',
  setActiveOnRegister:false
});

/* Dropdown menus */

class DropdownMenu {
  constructor(id) {
    this.id = id;
    this.container = document.getElementById('dropdown-'+id);
    this.container.querySelectorAll('ul li button').forEach(item => this.attach(item));
  }

  /*private*/ attach(item) {
    item.addEventListener('click', () => {
      this.removeActive();
      item.classList.add('active');
      if(this.onclick) {
        this.onclick(item.dataset[this.id]);
      }
    });
  }

  /*private*/ removeActive() {
    this.container.querySelectorAll('ul li button.active').forEach(item => item.classList.remove('active'));
  }

  /*private*/ getItem(value) {
    return this.container.querySelector('ul li button[data-'+this.id+'="'+value+'"]');
  }

  get() {
    let item = this.container.querySelector('ul li button.active');
    return item ? item.dataset[this.id] : null;
  }

  set(value) {
    this.removeActive();
    let item = this.getItem(value);
    if(item) item.classList.add('active');
  }

  add(value, text) {
    let li = document.createElement('li');
    let button = document.createElement('button');
    button.className="dropdown-item";
    button.type="button";
    button.dataset[this.id] = value;
    button.innerText = text;
    li.appendChild(button);
    this.container.querySelector('ul').appendChild(li);
    this.attach(button);
  }

  addDivider() {
    let li = document.createElement('li');
    let hr = document.createElement('hr');
    hr.className = 'dropdown-divider';
    li.appendChild(hr);
    this.container.querySelector('ul').appendChild(li);
  }

  remove(value) {
    let item = this.getItem(value);
    if(item) {
      item.parentElement.parentNode.removeChild(item.parentElement);
    }
  }

  removeAll() {
    this.container.querySelector('ul').innerHTML = '';
  }
}

/* Keyboard Dropdown */

const keyboardDropdown = new DropdownMenu('keyboard');
keyboardDropdown.onclick = (value) => {
  console.log('setting keyboard to '+value);
  keyman.setActiveKeyboard(value, 'en');
  ta1.focus();
  refreshStatusKeyboard(value);
};

function buildKeyboardList() {
  let keyboards = keyman.getKeyboards();

  keyboardDropdown.removeAll();
  keyboardDropdown.add('', '(system keyboard)');

  for(let keyboard of keyboards) {
    keyboardDropdown.add(keyboard.InternalName, keyboard.Name);
  }
  keyboardDropdown.set(keyman.getActiveKeyboard());
}



/* TODO: once KeymanWeb supports oninput signalling
document.getElementById('ta1').addEventListener('input', logContent, false);
*/

window.onload = function() {
  window.setTimeout(
    function () {
      keyman.moveToElement('ta1');
    }, 10
  );

  let newOSK = null;
  let deviceDropdown = null;
  let currentDevice = null;

  buildKeyboardList();

  if(!keyman.util.isTouchDevice()) {
    deviceDropdown = new DropdownMenu('device');
    currentDevice = window.sessionStorage.getItem('current-device');
    if(!devices[currentDevice]) currentDevice = 'Windows';
    deviceDropdown.set(currentDevice);
    deviceDropdown.onclick = (value) => {
      currentDevice = value;
      setOSK();
      ta1.focus();
      window.sessionStorage.setItem('current-device', currentDevice);
      refreshStatusDevice(currentDevice);
    };
    refreshStatusDevice(currentDevice);
  } else {
    document.body.classList.add('touch-device');
  }

  function setOSK() {
    if(keyman.util.isTouchDevice()) {
      return;
    }

    const targetDevice = devices[currentDevice] || devices.Windows;

    document.getElementById('osk-host-frame').className = currentDevice || 'Windows';

    if(newOSK) {
      document.getElementById('osk-host').removeChild(newOSK.element);
      keyman.osk = null;
    }

    newOSK = new com.keyman.osk.InlinedOSKView(targetDevice, keyman.util.device.coreSpec);

    if(document.body.offsetWidth < targetDevice.dimensions[0]) {
      newOSK.setSize('320px', '200px');
    } else {
      newOSK.setSize(targetDevice.dimensions[0]+'px', targetDevice.dimensions[1]+'px');
    }
    document.getElementById('osk-host').appendChild(newOSK.element);
  }

  setOSK();

  keyman.addEventListener('keyboardchange', function(keyboardProperties) {
    if(newOSK) {
      keyman.osk = newOSK;
      newOSK.activeKeyboard = keyman.core.activeKeyboard;
    }
    keyboardDropdown.set(keyboardProperties.internalName);
    window.sessionStorage.setItem('current-keyboard', keyboardProperties.internalName);
    keyman.alignInputs();
  });
}

function refreshStatusKeyboard(keyboard, model) {
  let statusKeyboard = document.getElementById('status-keyboard');
  let activeKeyboard = keyman.core.activeKeyboard ? keyman.core.activeKeyboard.id : '';
  let keyboards = keyman.getKeyboards();
  for(let k of keyboards) {
    if(k.InternalName == keyboard) activeKeyboard = k.Name;
  }
  statusKeyboard.innerText = activeKeyboard;
}

function refreshStatusModel(model) {
  let statusModel = document.getElementById('status-model');
  const activeModel = keyman.core.activeModel ? keyman.core.activeModel.id : '';
  model = model ? model : activeModel;
  statusModel.innerText = activeModel;
}

function refreshStatusDevice(device) {
  document.getElementById('status-device').innerText = device;
}

/* Refresh keyboard and model dropdowns */

let currentKeyboardModelScript = null;

function unloadKeyboardsAndModels() {
  let keyboards = keyman.getKeyboards().map(e => e.Name);
  console.log('Unregistering keyboards: '+JSON.stringify(keyboards));
  for(let keyboard of keyboards) {
    keyman.removeKeyboards(keyboard);
    if(window['Keyboard_'+keyboard]) {
      // can't `delete` as it is defined as a function rather than as a property
      window['Keyboard_'+keyboard] = undefined;
    }
  }

  const lastModel = keyman.core.activeModel;
  if(lastModel) {
    console.log('Unregistering model '+lastModel.id);
    keyman.modelManager.deregister(lastModel.id);
  }

  modelDropdown.removeAll();
  modelDropdown.add('', '(no model)');
}

function checkKeyboardsAndModels(shouldReload) {
  var req=new XMLHttpRequest();
  console.log('Checking for updated keyboards and models ('+shouldReload+')');
  req.onreadystatechange = function() {
    if (req.readyState==4) {
      if (req.status==200) {
        if(req.responseText !== currentKeyboardModelScript) {
          currentKeyboardModelScript = req.responseText;
          // we need to force a reload of the keyboard and model
          const currentKeyboard = !shouldReload ? window.sessionStorage.getItem('current-keyboard') : keyman.getActiveKeyboard();
          const lastModel = !shouldReload ? window.sessionStorage.getItem('current-model') : (keyman.core.activeModel ? keyman.core.activeModel.id : '');
          unloadKeyboardsAndModels();
          //console.log(req.responseText);
          eval(req.responseText);
          window.setTimeout(() => {
            console.log('setting active keyboard to '+currentKeyboard);
            console.log('setting active model to '+lastModel);
            keyman.setActiveKeyboard(currentKeyboard, 'en')
            selectModel(lastModel);
            refreshStatusKeyboard(currentKeyboard);
            refreshStatusModel(lastModel);
          }, 10);
        }
      }
    }
  }
  req.open("GET", "inc/keyboards.js", true);
  req.send(null);
}

checkKeyboardsAndModels(false);

/* Lexical models */

const modelDropdown = new DropdownMenu('model');
modelDropdown.onclick = selectModel;

/**
 * Register a model for debugging. Called by keyboards.js. The
 * first model registered will be activated automatically.
 */
function registerModel(model, src) {
  //console.log('modelRegistered: '+model);
  registeredModels[model] = {id: model, src: src};
  modelDropdown.add(model, model);
}

/**
 * Select a model by id
 * @param {string} modelId
 * @returns void
 */
function selectModel(modelId) {
  const model = registeredModels[modelId];
  const lastModel = keyman.core.activeModel;

  if(lastModel) {
    keyman.modelManager.deregister(lastModel.id);
  }

  if(model) {
    keyman.modelManager.register({
      id: model.id,
      languages: ['en'],
      path: location.protocol + '//' + location.host + '/data/model/' + model.src
    });
  }
  window.sessionStorage.setItem('current-model', model ? model.id : '');
  modelDropdown.set(model ? model.id : '');
  refreshStatusModel(modelId);
  ta1.focus();
}

