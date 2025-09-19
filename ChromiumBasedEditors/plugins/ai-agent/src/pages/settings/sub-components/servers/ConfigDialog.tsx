import React from "react";
import { EditorView, basicSetup } from "codemirror";
import { json } from "@codemirror/lang-json";
import { EditorState } from "@codemirror/state";
import { ViewUpdate } from "@codemirror/view";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";
import useServersStore from "@/store/useServersStore";

type ConfigDialogProps = {
  open: boolean;
  onClose: () => void;
};

const ConfigDialog = ({ open, onClose }: ConfigDialogProps) => {
  const { saveConfig, getConfig } = useServersStore();

  const editorRef = React.useRef<HTMLDivElement>(null);
  const viewRef = React.useRef<EditorView | null>(null);
  const [value, setValue] = React.useState("");
  const [error, setError] = React.useState("");
  const [isValidJson, setIsValidJson] = React.useState(true);

  const validateJson = (jsonString: string) => {
    try {
      JSON.parse(jsonString);
      setError("");
      setIsValidJson(true);
      return true;
    } catch (err) {
      setError(
        `Invalid JSON: ${err instanceof Error ? err.message : "Unknown error"}`
      );
      setIsValidJson(false);
      return false;
    }
  };

  React.useEffect(() => {
    if (!open) return;

    const initializeEditor = () => {
      if (editorRef.current && !viewRef.current) {
        const initialDoc = JSON.stringify(getConfig(), null, 2);

        setValue(initialDoc);
        validateJson(initialDoc);

        const state = EditorState.create({
          doc: initialDoc,
          extensions: [
            basicSetup,
            json(),
            EditorView.updateListener.of((update: ViewUpdate) => {
              if (update.docChanged) {
                const newValue = update.state.doc.toString();
                setValue(newValue);
                validateJson(newValue);
              }
            }),
            EditorView.theme({
              "&": {
                height: "400px",
                maxHeight: "400px",
              },
              ".cm-content": {
                padding: "10px",
              },
              ".cm-focused": {
                outline: "none",
              },
              ".cm-editor": {
                height: "400px",
                maxHeight: "400px",
              },
              ".cm-scroller": {
                fontFamily: "monospace",
                height: "400px",
                maxHeight: "400px",
              },
              ".cm-gutters": {
                height: "400px",
                maxHeight: "400px",
              },
            }),
          ],
        });

        viewRef.current = new EditorView({
          state,
          parent: editorRef.current,
        });
      } else {
        setTimeout(initializeEditor, 100);
      }
    };

    setTimeout(initializeEditor, 50);

    return () => {
      if (viewRef.current) {
        viewRef.current.destroy();
        viewRef.current = null;
      }
    };
  }, [open, getConfig]);

  return (
    <Dialog open={open} onOpenChange={() => {}}>
      <DialogContent header="Config" onClose={onClose} isAside={false}>
        <div className="flex flex-col gap-[16px] h-full">
          <p className="text-sm text-gray-600">Edit your JSON configuration:</p>
          <div
            ref={editorRef}
            className="border border-gray-300 rounded-md overflow-hidden h-[400px] max-h-[400px]"
          />
          {error && (
            <div className="text-red-600 text-sm bg-red-50 border border-red-200 rounded-md p-2">
              {error}
            </div>
          )}
        </div>
        <div className="flex flex-row gap-[8px] mt-[16px]">
          <Button onClick={onClose} variant="default">
            Cancel
          </Button>
          <Button
            disabled={!isValidJson}
            onClick={() => {
              saveConfig(JSON.parse(value));
              onClose();
            }}
          >
            Save
          </Button>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export default ConfigDialog;
