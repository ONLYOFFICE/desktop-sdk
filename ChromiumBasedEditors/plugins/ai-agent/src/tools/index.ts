import { DesktopEditorTool } from "./DesktopEditor";

class Tools {
  desktopEditorTool: DesktopEditorTool;

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
  }

  init = () => {
    this.desktopEditorTool.initTools();
  };

  getTools = () => {
    return this.desktopEditorTool.getTools();
  };

  callTools = (name: string, args: Record<string, unknown>) => {
    return this.desktopEditorTool.callTools(name, args);
  };
}

export default Tools;
