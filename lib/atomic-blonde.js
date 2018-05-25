'use babel';
'use strict';

import AtomicBlondeView from './atomic-blonde-view';
import { CompositeDisposable } from 'atom';

const blonde = require('../build/Release/blonde')

var tree = undefined;

export function activate(state) {
    // initialize sourcekit 
    blonde.initialize();
    
    // initialize object reference manager
    tree = new Tree();
    console.log("activated");
}

export function deactivate() {
    // deinitialize object reference manager
    tree.subscriptions.dispose();
    tree = undefined;
    
    // deinitialize sourcekit 
    blonde.deinitialize();
    
    console.log("deactivated");
}

export function serialize() {
    return {};
}

class Tree {
    constructor() {
        // holds references to each witness so they can delete themselves when closed
        this.witnesses = new Set();
        this.subscriptions = new CompositeDisposable();
        
        this.subscriptions.add(atom.workspace.observeTextEditors((editor) => {
            this.witnesses.add(new EditorWitness(editor, {
                dispose: (witness) => {
                    this.witnesses.delete(witness);
                }
            }))
        }));
    }
}

class EditorWitness {
    constructor(editor, callbacks) {
        console.log("new editor");
        this.editor             = editor;
        this.subscriptions = new CompositeDisposable();
        
        // members that are only defined while the editor is a Swift editor 
        this.swift         = undefined;
        
        this.subscriptions.add(
            editor.onDidChangeGrammar((grammar) => this.updateGrammar(grammar)), 
            editor.onDidDestroy(() => {
                this.subscriptions.dispose();
                this.swiftClear();
                callbacks.dispose(this);
            })
        );
        
        this.updateGrammar(editor.getGrammar());
    }
    
    updateGrammar(grammar) {
        if (grammar.scopeName == 'source.swift') {
            this.swiftInit();
        }
        else {
            this.swiftClear();
        }
    }
    
    swiftInit() {
        if (this.swift === undefined) {
            this.swift = {
                subscriptions: new CompositeDisposable(), 
                markerLayer: this.editor.addMarkerLayer()
            }
            
            this.swift.subscriptions.add(
                this.editor.onDidStopChanging(() => {
                    console.log('highlight');
                    this.highlight();
                })
            );
            
            console.log("subscribed");
        }
    }
    
    swiftClear() {
        if (this.swift !== undefined) 
        {
            console.log("unsubscribing");
            this.swift.subscriptions.dispose();
            this.swift.markerLayer.clear();
            this.swift = undefined;
        }
    }
    
    highlight() {
        let source = this.editor.getText();
        
        const tokenBuffer = blonde.highlight(source);
        const count = tokenBuffer.length >> 3;
        console.log(source.length, count, tokenBuffer.length);
        this.swift.markerLayer.clear();
        for (let i = 0; i < count; ++i)
        {
            let ay = tokenBuffer.readUInt16LE(i << 3    );
            let ax = tokenBuffer.readUInt16LE(i << 3 | 2);
            let by = tokenBuffer.readUInt16LE(i << 3 | 4);
            let bx = tokenBuffer.readUInt16LE(i << 3 | 6);
            console.log('[' + i + ']: ' + ay + ' ' + ax + ' | ' +  by + ' ' + bx);
            this.swift.markerLayer.markBufferRange([[ay, ax], [by, bx]]);
        }
        
        this.editor.decorateMarkerLayer(this.swift.markerLayer, {
            type: 'text', 'class': 'syntax--keyword'
        });
    }
}
