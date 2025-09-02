import { DesktopEditorTool } from "./DesktopEditor";

class Servers {
  desktopEditorTool: DesktopEditorTool;

  constructor() {
    this.desktopEditorTool = new DesktopEditorTool();
  }

  init = () => {
    this.desktopEditorTool.initTools();
  };

  getTools = async () => {
    const [desktopEditorTools] = await Promise.all([
      this.desktopEditorTool.getTools(),
    ]);

    return {
      "Desktop Editor": desktopEditorTools,
    };
  };

  callTools = (name: string, args: Record<string, unknown>) => {
    return this.desktopEditorTool.callTools(name, args);
  };
}

const servers = new Servers();

export default servers;
