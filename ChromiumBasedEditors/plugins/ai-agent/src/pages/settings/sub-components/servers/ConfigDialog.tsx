import React from "react";
import { useTranslation } from "react-i18next";

import { EditorView, basicSetup } from "codemirror";
import { json } from "@codemirror/lang-json";
import { EditorState } from "@codemirror/state";
import { ViewUpdate } from "@codemirror/view";

import { Dialog, DialogContent } from "@/components/dialog";
import { Button } from "@/components/button";
import useServersStore from "@/store/useServersStore";

import "./ConfigDialog.css";

type ConfigDialogProps = {
  open: boolean;
  onClose: () => void;
};

const ConfigDialog = ({ open, onClose }: ConfigDialogProps) => {
  const { t } = useTranslation();
  const { saveConfig, getConfig } = useServersStore();

  const editorRef = React.useRef<HTMLDivElement>(null);
  const viewRef = React.useRef<EditorView | null>(null);
  const [value, setValue] = React.useState("");
  const [error, setError] = React.useState("");
  const [isValidJson, setIsValidJson] = React.useState(true);

  const validateJson = React.useCallback(
    (jsonString: string) => {
      try {
        const parsed = JSON.parse(jsonString);
        if (parsed.mcpServers) {
          setIsValidJson(true);
          setError("");
          return true;
        }

        setIsValidJson(false);
        setError(t("ConfigurationError"));

        return false;
      } catch (err) {
        setError(
          `Invalid JSON format\n ${
            err instanceof Error ? err.message : "Unknown error"
          }`
        );
        setIsValidJson(false);
        return false;
      }
    },
    [t]
  );

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
                height: "200px",
                maxHeight: "200px",
              },
              ".cm-content": {
                padding: "10px",
              },
              ".cm-focused": {
                outline: "none",
              },
              ".cm-editor": {
                height: "200px",
                maxHeight: "200px",
              },
              ".cm-scroller": {
                fontFamily: "monospace",
                height: "200px",
                maxHeight: "200px",
              },
              ".cm-gutters": {
                height: "200px",
                maxHeight: "200px",
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
  }, [open, t, validateJson, getConfig]);

  const onSubmitAction = React.useCallback(() => {
    if (!isValidJson) return;
    saveConfig(JSON.parse(value));
    onClose();
  }, [isValidJson, saveConfig, value, onClose]);

  React.useEffect(() => {
    if (!open) return;

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) {
        e.preventDefault();
        onSubmitAction();
      }
    };

    window.addEventListener("keydown", handleKeyDown);

    return () => {
      window.removeEventListener("keydown", handleKeyDown);
    };
  }, [open, onSubmitAction]);

  return (
    <Dialog open={open} onOpenChange={() => {}}>
      <DialogContent
        header={t("EditConfiguration")}
        onClose={onClose}
        className="w-[564px] min-h-[400px]"
      >
        <div className="flex flex-col gap-[8px] min-h-[280px] pt-[8px] pb-[16px]">
          <div className="flex flex-col gap-[8px] min-h-[256px] p-[12px] bg-[var(--servers-edit-config-json-background-color)] rounded-[12px]">
            <div className="flex flex-row justify-between">
              <p className="font-bold text-[14px] leading-[20px] text-[var(--servers-edit-config-json-header-color)]">
                {t("EnterYourJSONConfiguration")}
              </p>
              <p className="font-normal text-[14px] leading-[20px] text-[var(--servers-edit-config-json-lang-color)]">
                json
              </p>
            </div>
            <div
              ref={editorRef}
              className="border border-[var(--servers-edit-config-json-editor-border-color)] bg-[var(--servers-edit-config-json-editor-background-color)] rounded-[4px] overflow-hidden h-full max-h-full"
            />
            {error ? (
              <p className="text-[var(--text-negative)] font-normal text-[14px] leading-[20px] whitespace-pre-line">
                {error}
              </p>
            ) : null}
          </div>
        </div>
        <div className="flex flex-row items-center justify-end gap-[16px] h-[64px] border-t border-[var(--servers-edit-config-buttons-border-color)] mx-[-32px] px-[32px]">
          <Button onClick={onClose} variant="default">
            {t("Cancel")}
          </Button>
          <Button disabled={!isValidJson} onClick={onSubmitAction}>
            {t("Save")}
          </Button>
        </div>
      </DialogContent>
    </Dialog>
  );
};

export default ConfigDialog;
