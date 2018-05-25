'use babel';
'use strict';

import AtomicBlondeView from './atomic-blonde-view';
import { CompositeDisposable } from 'atom';

const blonde = require('../build/Release/blonde')

// need to clean up markers after each iteration
function highlight(editor) {
    console.log('highlight');
    let source = editor.getText();
    
    console.log(blonde.testfunc());
    
    let markerLayer = editor.addMarkerLayer();
    
    const tokenBuffer = blonde.highlight(source);
    const count = tokenBuffer.length >> 3;
    console.log(count, tokenBuffer.length);
    for (let i = 0; i < count; ++i)
    {
        let ay = tokenBuffer.readUInt16LE(i << 3    );
        let ax = tokenBuffer.readUInt16LE(i << 3 | 2);
        let by = tokenBuffer.readUInt16LE(i << 3 | 4);
        let bx = tokenBuffer.readUInt16LE(i << 3 | 6);
        console.log(i, ':', ay, ax, by, bx);
        markerLayer.markBufferRange([[ay, ax], [by, bx]]);
    }

    editor.decorateMarkerLayer(markerLayer, {
        type: 'text', 'class': 'syntax--keyword'
    })
}

export default {
    atomicBlondeView: null,
    modalPanel: null,
    subscriptions: null,

    activate(state) {
        this.atomicBlondeView = new AtomicBlondeView(state.atomicBlondeViewState);
        this.modalPanel = atom.workspace.addModalPanel({
            item: this.atomicBlondeView.getElement(),
            visible: false
        });

        // Events subscribed to in atom's system can be easily cleaned up with a CompositeDisposable
        this.subscriptions = new CompositeDisposable();

        // initialize sourcekit 
        blonde.initialize();
        
        // Register command that toggles this view
        this.subscriptions.add(atom.commands.add('atom-workspace', {
            'atomic-blonde:enable': () => this.enable()
        }));
    },

    deactivate() {
        this.modalPanel.destroy();
        this.subscriptions.dispose();
        this.atomicBlondeView.destroy();
        
        // deinitialize sourcekit 
        blonde.deinitialize();
    },

    serialize() {
        return {
            atomicBlondeViewState: this.atomicBlondeView.serialize()
        };
    },

    enable() {
        let editor = atom.workspace.getActiveTextEditor();
        editor.onDidStopChanging(() => highlight(editor));
        //editor.onDidDestroy(() => highlight(editor));
    }
};
