import React from "react";
import { useTranslation } from "react-i18next";
import { ReactSVG } from "react-svg";

import AttachmentIconUrl from "@/assets/attachment.svg?url";
import DocumentsIconSvg from "@/assets/formats/24/documents.svg?url";
import SpreadsheetsIconSvg from "@/assets/formats/24/spreadsheets.svg?url";
import PdfIconSvg from "@/assets/formats/24/pdf.svg?url";
import PresentationsIconSvg from "@/assets/formats/24/presentations.svg?url";

import useAttachmentsStore from "@/store/useAttachmentsStore";
import useProviders from "@/store/useProviders";
import useModelsStore from "@/store/useModelsStore";

import { isDocument, isPdf, isPresentation, isSpreadsheet } from "@/lib/utils";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";
import type { DropDownItemProps } from "@/components/dropdown-item/DropDownItem.types";
import { TooltipIconButton } from "@/components/tooltip-icon-button";

const ComposerActionAttachment = () => {
  const [isOpen, setIsOpen] = React.useState(false);

  const { addAttachmentFile } = useAttachmentsStore();
  const { currentProvider } = useProviders();
  const { currentModel } = useModelsStore();

  const onOpenChange = (open: boolean) => {
    setIsOpen(open);
  };

  const selectRecentFile = (path: string, type: number) => {
    window.AscDesktopEditor.convertFileExternal(path, 69, (data, error) => {
      if (error) {
        console.log("Error:", error);
        return;
      }

      const uint8Array = new Uint8Array(data.content);
      const textDecoder = new TextDecoder("utf-8");
      const stringData = textDecoder.decode(uint8Array);

      addAttachmentFile({ path, content: stringData, type });
    });
  };

  const selectLocalFile = () => {
    window.AscDesktopEditor.OpenFilenameDialog(
      "*.docx , *.pdf , *.xlsx",
      true,
      (file) => {
        if (Array.isArray(file)) {
          file.forEach((file, index) => {
            if (index > 5) return;

            const extension = file.split(".").pop() ?? "";

            window.AscDesktopEditor.convertFileExternal(
              file,
              69,
              (data, error) => {
                if (error) {
                  console.log("Error:", error);
                  return;
                }

                const uint8Array = new Uint8Array(data.content);
                const textDecoder = new TextDecoder("utf-8");
                const stringData = textDecoder.decode(uint8Array);

                addAttachmentFile({
                  path: file,
                  content: stringData || "",
                  type:
                    extension === "pdf" ? 87 : extension === "docx" ? 65 : 257,
                });
              }
            );
          });
        }
      }
    );
  };

  const recentFiles = (
    JSON.parse(
      window.AscDesktopEditor?.callToolFunction("recent_files_reader") ?? "{}"
    ) as { files: { path: string; type: number; url: string }[] }
  )?.files
    ?.filter((file) => !file.url)
    ?.map((file) => {
      let icon: null | string = DocumentsIconSvg;

      if (isPdf(file.type)) {
        icon = PdfIconSvg;
      } else if (isSpreadsheet(file.type)) {
        icon = SpreadsheetsIconSvg;
      } else if (isDocument(file.type)) {
        icon = DocumentsIconSvg;
      } else if (isPresentation(file.type)) {
        icon = PresentationsIconSvg;
      } else {
        icon = null;
      }

      return {
        text: file.path.includes("\\")
          ? file.path.split("\\").pop() ?? ""
          : file.path.split("/").pop() ?? "",
        key: file.path,
        id: file.path,
        icon: icon ? <ReactSVG src={icon} /> : null,
        onClick: () => selectRecentFile(file.path, file.type),
      };
    })
    .filter(Boolean);

  const { t } = useTranslation();

  const trigger = (
    <TooltipIconButton tooltip={t("Attachments")} visible={!isOpen}>
      <IconButton
        iconName={AttachmentIconUrl}
        size={24}
        className="cursor-pointer rounded-[4px] outline-none"
        isStroke
        isActive={isOpen}
        disabled={!currentProvider || !currentModel}
      />
    </TooltipIconButton>
  );

  const items: DropDownItemProps[] = [
    { text: t("AddLocalFile"), onClick: () => selectLocalFile() },
    { text: "", onClick: () => {}, isSeparator: true },
    { text: t("RecentFiles"), onClick: () => {}, subMenu: recentFiles },
  ];

  return (
    <DropdownMenu trigger={trigger} items={items} onOpenChange={onOpenChange} />
  );
};

export { ComposerActionAttachment };
