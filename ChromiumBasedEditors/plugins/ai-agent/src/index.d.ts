import type { TProcess } from "./lib/types";

export {};

declare global {
  interface Window {
    AscSimpleRequest: {
      createRequest: (options: {
        url: string;
        method: string;
        headers: Record<string, string>;
        body: string;
        complete: (e: { responseText: string }) => void;
        error: (e: { statusCode: number }) => void;
      }) => void;
    };
    AscDesktopEditor: {
      getToolFunctions: () => string;
      callToolFunction: (name: string, args?: string) => string;
      openTemplate: (file: string, name: string) => void;
      OpenFilenameDialog: (
        type: string,
        multiple: boolean,
        callback: (path: string | string[]) => void
      ) => void;
      convertFileExternal: (
        file: string,
        type: number,
        callback: (data: { content: Uint8Array }, error: unknown) => void
      ) => void;
      SaveFilenameDialog: (
        fileName: string,
        callback: (path: string) => void,
        content: string
      ) => void;
    };
    RendererProcessVariable: {
      lang: string;
      rtl: boolean;
      theme: {
        system: string;
        type: string;
        id: string;
        addLocal: "on" | "off";
      };
    };
    on_update_plugin_info: (info: { theme: string; lang: string }) => void;
    ExternalProcess: new (
      command: string,
      env?: Record<string, string>
    ) => TProcess;
  }
}
