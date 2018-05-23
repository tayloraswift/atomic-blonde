'use babel';

import AtomicBlonde from '../lib/atomic-blonde';

// Use the command `window:run-package-specs` (cmd-alt-ctrl-p) to run specs.
//
// To run a specific `it` or `describe` block add an `f` to the front (e.g. `fit`
// or `fdescribe`). Remove the `f` to unfocus the block.

describe('AtomicBlonde', () => {
  let workspaceElement, activationPromise;

  beforeEach(() => {
    workspaceElement = atom.views.getView(atom.workspace);
    activationPromise = atom.packages.activatePackage('atomic-blonde');
  });

  describe('when the atomic-blonde:toggle event is triggered', () => {
    it('hides and shows the modal panel', () => {
      // Before the activation event the view is not on the DOM, and no panel
      // has been created
      expect(workspaceElement.querySelector('.atomic-blonde')).not.toExist();

      // This is an activation event, triggering it will cause the package to be
      // activated.
      atom.commands.dispatch(workspaceElement, 'atomic-blonde:toggle');

      waitsForPromise(() => {
        return activationPromise;
      });

      runs(() => {
        expect(workspaceElement.querySelector('.atomic-blonde')).toExist();

        let atomicBlondeElement = workspaceElement.querySelector('.atomic-blonde');
        expect(atomicBlondeElement).toExist();

        let atomicBlondePanel = atom.workspace.panelForItem(atomicBlondeElement);
        expect(atomicBlondePanel.isVisible()).toBe(true);
        atom.commands.dispatch(workspaceElement, 'atomic-blonde:toggle');
        expect(atomicBlondePanel.isVisible()).toBe(false);
      });
    });

    it('hides and shows the view', () => {
      // This test shows you an integration test testing at the view level.

      // Attaching the workspaceElement to the DOM is required to allow the
      // `toBeVisible()` matchers to work. Anything testing visibility or focus
      // requires that the workspaceElement is on the DOM. Tests that attach the
      // workspaceElement to the DOM are generally slower than those off DOM.
      jasmine.attachToDOM(workspaceElement);

      expect(workspaceElement.querySelector('.atomic-blonde')).not.toExist();

      // This is an activation event, triggering it causes the package to be
      // activated.
      atom.commands.dispatch(workspaceElement, 'atomic-blonde:toggle');

      waitsForPromise(() => {
        return activationPromise;
      });

      runs(() => {
        // Now we can test for view visibility
        let atomicBlondeElement = workspaceElement.querySelector('.atomic-blonde');
        expect(atomicBlondeElement).toBeVisible();
        atom.commands.dispatch(workspaceElement, 'atomic-blonde:toggle');
        expect(atomicBlondeElement).not.toBeVisible();
      });
    });
  });
});
