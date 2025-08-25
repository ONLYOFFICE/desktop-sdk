export {};

declare global {
  interface Window {
    AscDesktopEditor: {
      getToolFunctions: () => string;
      callToolFunction: (name: string, args: string) => string;
      openTemplate: (file: string, name: string) => void;
    };
  }
}
