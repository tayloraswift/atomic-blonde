'use babel';
'use strict';

import AtomicBlondeView from './atomic-blonde-view';
import { CompositeDisposable } from 'atom';

const blonde = require('../build/Release/blonde')

const speciesClasses = [
    'syntax--keyword', 
    'syntax--identifier', 
    'syntax--entity syntax--name syntax--type', 
    
    'syntax--keyword syntax--preprocessor syntax--directive', 
    'syntax--identifier syntax--preprocessor', 
    'syntax--keyword syntax--pounddirective-keyword', 
    
    'syntax--attribute-id', 
    'syntax--entity syntax--other syntax--attribute-name syntax--attribute-builtin', 
    
    'syntax--constant syntax--numeric', 
    'syntax--string', 
    'syntax--variable syntax--interpolation syntax--string-interpolation-anchor', 
    
    'syntax--comment', 
    'syntax--comment syntax--doccomment', 
    'syntax--comment syntax--doccomment syntax--doccomment-field', 
    'syntax--comment syntax--comment-mark', 
    'syntax--comment syntax--markup syntax--link', 
    
    'syntax--placeholder', 
    'syntax--objectliteral'
]

var tree = undefined;

export function activate(state) {
    // initialize sourcekit 
    blonde.initialize();
    
    // initialize object reference manager
    tree = new Tree();
}

export function deactivate() {
    // deinitialize object reference manager
    tree.subscriptions.dispose();
    tree = undefined;
    
    // deinitialize sourcekit 
    blonde.deinitialize();
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
        this.editor        = editor;
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
                markerLayers: [...Array(speciesClasses.length)].map(_ => this.editor.addMarkerLayer()), 
                decorationLayers: undefined
            }
            
            this.highlight();
            
            this.swift.subscriptions.add(
                this.editor.onDidChange(() => {
                    this.highlight();
                })
            );
        }
    }
    
    swiftClear() {
        if (this.swift !== undefined) 
        {
            this.swift.subscriptions.dispose();
            this.swiftClearMarkings();
            this.swift = undefined;
        }
    }
    
    swiftClearMarkings() {
        if (this.swift.decorationLayers !== undefined) {
            this.swift.decorationLayers.map((layer, _) => layer.destroy());
            this.swift.decorationLayers = undefined;
        }
        this.swift.markerLayers.map((layer, _) => layer.clear());
    }
    
    highlight() {
        let source = this.editor.getText();
        
        const tokenBuffer = blonde.highlight(source);
        const limit       = tokenBuffer.length;
        this.swiftClearMarkings();
        let b = 0;
        while (b < limit)
        {
            // will probably break on big-endian systems but Nan::Buffer doesn’t 
            // seem to be able to use the platform endianess
            let ay      = tokenBuffer.readUInt16LE(b    );
            let ax      = tokenBuffer.readUInt16LE(b + 2);
            let by      = tokenBuffer.readUInt16LE(b + 4);
            let bx      = tokenBuffer.readUInt16LE(b + 6);
            let species = tokenBuffer[b + 8];
            //console.log('[' + i + ']: (' + ay + ', ' + ax + ') -> (' + by + ', ' + bx + ') ' + speciesClasses[species]);
            this.swift.markerLayers[species].markBufferRange([[ay, ax], [by, bx]]);
            b += 10;
        }
        
        this.swift.decorationLayers = this.swift.markerLayers.map((markerLayer, i) =>
                this.editor.decorateMarkerLayer(markerLayer, {
                    type: 'text', 'class': speciesClasses[i]
                })
            );
    }
}
