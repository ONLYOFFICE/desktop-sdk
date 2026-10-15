import { beforeEach, describe, expect, it, vi } from "vitest";
import { DesktopEditorTool } from "../DesktopEditor";

// =============================================================================
// Mock Setup
// =============================================================================

const mockGetToolFunctions = vi.fn();
const mockCallToolFunction = vi.fn();

const mockWindow = {
  AscDesktopEditor: {
    getToolFunctions: mockGetToolFunctions,
    callToolFunction: mockCallToolFunction,
  },
};

vi.stubGlobal("window", mockWindow);

describe("DesktopEditorTool", () => {
  let desktopEditor: DesktopEditorTool;

  beforeEach(() => {
    vi.clearAllMocks();
    mockGetToolFunctions.mockReturnValue("[]");
  });

  // ==========================================================================
  // Constructor & Initialization
  // ==========================================================================

  describe("constructor", () => {
    it("should initialize with empty tools when getToolFunctions returns empty array", () => {
      mockGetToolFunctions.mockReturnValue("[]");

      desktopEditor = new DesktopEditorTool();

      expect(desktopEditor.getTools()).toEqual([]);
    });

    it("should initialize tools from AscDesktopEditor", () => {
      const mockTools = [
        {
          name: "read_file",
          description: "Read a file",
          parameters: {
            type: "object",
            properties: { path: { type: "string" } },
          },
        },
        {
          name: "write_file",
          description: "Write to a file",
          parameters: {
            type: "object",
            properties: { path: { type: "string" } },
          },
        },
      ];
      mockGetToolFunctions.mockReturnValue(JSON.stringify(mockTools));

      desktopEditor = new DesktopEditorTool();
      const tools = desktopEditor.getTools();

      expect(tools).toHaveLength(2);
      expect(tools[0]).toEqual({
        name: "read_file",
        description: "Read a file",
        inputSchema: {
          type: "object",
          properties: { path: { type: "string" } },
        },
      });
    });

    it("should keep the annotations of a desktop function", () => {
      const mockTools = [
        {
          name: "run_script",
          description: "Run a script",
          parameters: { type: "object", properties: {} },
          annotations: { readOnlyHint: false, destructiveHint: true },
        },
      ];
      mockGetToolFunctions.mockReturnValue(JSON.stringify(mockTools));

      desktopEditor = new DesktopEditorTool();

      expect(desktopEditor.getTools()[0].annotations).toEqual({
        readOnlyHint: false,
        destructiveHint: true,
      });
    });

    it("should handle missing AscDesktopEditor gracefully", () => {
      vi.stubGlobal("window", {});

      desktopEditor = new DesktopEditorTool();

      expect(desktopEditor.getTools()).toEqual([]);

      // Restore mock
      vi.stubGlobal("window", mockWindow);
    });

    it("should handle invalid JSON gracefully", () => {
      const consoleSpy = vi.spyOn(console, "error").mockImplementation(vi.fn());
      mockGetToolFunctions.mockReturnValue("invalid json");

      desktopEditor = new DesktopEditorTool();

      expect(desktopEditor.getTools()).toEqual([]);
      expect(consoleSpy).toHaveBeenCalledWith(
        "Error parsing tools:",
        expect.any(Error)
      );

      consoleSpy.mockRestore();
    });
  });

  // ==========================================================================
  // setTools
  // ==========================================================================

  describe("setTools", () => {
    beforeEach(() => {
      desktopEditor = new DesktopEditorTool();
    });

    it("should set tools", () => {
      const tools = [
        { name: "tool1", description: "desc1", inputSchema: {} },
        { name: "tool2", description: "desc2", inputSchema: {} },
      ];

      desktopEditor.setTools(tools);

      expect(desktopEditor.getTools()).toEqual(tools);
    });

    it("should replace existing tools", () => {
      desktopEditor.setTools([
        { name: "old", description: "old", inputSchema: {} },
      ]);
      desktopEditor.setTools([
        { name: "new", description: "new", inputSchema: {} },
      ]);

      expect(desktopEditor.getTools()).toHaveLength(1);
      expect(desktopEditor.getTools()[0].name).toBe("new");
    });
  });

  // ==========================================================================
  // getTools
  // ==========================================================================

  describe("getTools", () => {
    beforeEach(() => {
      desktopEditor = new DesktopEditorTool();
    });

    it("should return empty array when no tools", () => {
      expect(desktopEditor.getTools()).toEqual([]);
    });

    it("should return a copy of tools array", () => {
      desktopEditor.setTools([
        { name: "tool", description: "desc", inputSchema: {} },
      ]);

      const tools1 = desktopEditor.getTools();
      const tools2 = desktopEditor.getTools();

      expect(tools1).not.toBe(tools2);
      expect(tools1).toEqual(tools2);
    });

    it("should not allow modification of internal tools array", () => {
      desktopEditor.setTools([
        { name: "original", description: "desc", inputSchema: {} },
      ]);

      const tools = desktopEditor.getTools();
      tools.push({ name: "added", description: "desc", inputSchema: {} });

      expect(desktopEditor.getTools()).toHaveLength(1);
      expect(desktopEditor.getTools()[0].name).toBe("original");
    });
  });

  // ==========================================================================
  // callTools
  // ==========================================================================

  describe("callTools", () => {
    beforeEach(() => {
      desktopEditor = new DesktopEditorTool();
    });

    it("should call AscDesktopEditor.callToolFunction with correct arguments", async () => {
      mockCallToolFunction.mockResolvedValue("result");

      await desktopEditor.callTools("read_file", { path: "/test/file.txt" });

      expect(mockCallToolFunction).toHaveBeenCalledWith(
        "read_file",
        JSON.stringify({ path: "/test/file.txt" })
      );
    });

    it("should return result from callToolFunction", async () => {
      mockCallToolFunction.mockResolvedValue("file content here");

      const result = await desktopEditor.callTools("read_file", {
        path: "/test.txt",
      });

      expect(result).toBe("file content here");
    });

    it("should handle empty args", async () => {
      mockCallToolFunction.mockResolvedValue("result");

      await desktopEditor.callTools("list_files", {});

      expect(mockCallToolFunction).toHaveBeenCalledWith("list_files", "{}");
    });

    it("should handle complex args", async () => {
      mockCallToolFunction.mockResolvedValue("result");

      const complexArgs = {
        path: "/test",
        options: { recursive: true, depth: 3 },
        filters: ["*.ts", "*.tsx"],
      };

      await desktopEditor.callTools("search", complexArgs);

      expect(mockCallToolFunction).toHaveBeenCalledWith(
        "search",
        JSON.stringify(complexArgs)
      );
    });

    it("should return undefined when AscDesktopEditor is not available", async () => {
      vi.stubGlobal("window", {});

      const result = await desktopEditor.callTools("test", {});

      expect(result).toBeUndefined();

      // Restore mock
      vi.stubGlobal("window", mockWindow);
    });

    it("should not mutate original args object", async () => {
      mockCallToolFunction.mockResolvedValue("result");

      const originalArgs = { path: "/test" };
      const argsCopy = { ...originalArgs };

      await desktopEditor.callTools("read_file", originalArgs);

      expect(originalArgs).toEqual(argsCopy);
    });
  });

  // ==========================================================================
  // initTools
  // ==========================================================================

  describe("initTools", () => {
    it("should map parameters to inputSchema", () => {
      const mockTools = [
        {
          name: "tool",
          description: "A tool",
          parameters: {
            type: "object",
            properties: {
              arg1: { type: "string" },
              arg2: { type: "number" },
            },
            required: ["arg1"],
          },
        },
      ];
      mockGetToolFunctions.mockReturnValue(JSON.stringify(mockTools));

      desktopEditor = new DesktopEditorTool();
      const tools = desktopEditor.getTools();

      expect(tools[0].inputSchema).toEqual({
        type: "object",
        properties: {
          arg1: { type: "string" },
          arg2: { type: "number" },
        },
        required: ["arg1"],
      });
    });

    it("should handle tools without parameters", () => {
      const mockTools = [
        {
          name: "no_params_tool",
          description: "Tool without params",
          parameters: {},
        },
      ];
      mockGetToolFunctions.mockReturnValue(JSON.stringify(mockTools));

      desktopEditor = new DesktopEditorTool();
      const tools = desktopEditor.getTools();

      expect(tools[0].inputSchema).toEqual({});
    });

    it("should handle empty string from getToolFunctions", () => {
      const consoleSpy = vi.spyOn(console, "error").mockImplementation(vi.fn());
      mockGetToolFunctions.mockReturnValue("");

      desktopEditor = new DesktopEditorTool();

      expect(desktopEditor.getTools()).toEqual([]);

      consoleSpy.mockRestore();
    });
  });
});
