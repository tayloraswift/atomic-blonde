'use babel';

import AtomicBlondeView from './atomic-blonde-view';
import { CompositeDisposable } from 'atom';

var ffi = require('ffi');

// need to clean up markers after each iteration
function highlight(editor) {
    console.log('highlight');
    let source = editor.getText();

    let marker = editor.markBufferRange([[0, 2], [0, 5]]);
    editor.decorateMarker(marker, {
        type: 'text', 'class': 'syntax--string'
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

        // Register command that toggles this view
        this.subscriptions.add(atom.commands.add('atom-workspace', {
            'atomic-blonde:enable': () => this.enable()
        }));


        atom.workspace.observeTextEditors(editor => {
            editor.onDidStopChanging(() => highlight(editor));
        })
    },

    deactivate() {
        this.modalPanel.destroy();
        this.subscriptions.dispose();
        this.atomicBlondeView.destroy();
    },

    serialize() {
        return {
            atomicBlondeViewState: this.atomicBlondeView.serialize()
        };
    },

    enable() {
    }
};
