import React from "react";
import { useTranslation } from "react-i18next";
import { ReactSVG } from "react-svg";

import AttachmentIconUrl from "@/assets/attachment.svg?url";
import DocumentsIconSvg from "@/assets/formats/24/documents.svg?url";
import SpreadsheetsIconSvg from "@/assets/formats/24/spreadsheets.svg?url";
import PdfIconSvg from "@/assets/formats/24/pdf.svg?url";

import useAttachmentsStore from "@/store/useAttachmentsStore";

import { IconButton } from "@/components/icon-button";
import { DropdownMenu } from "@/components/dropdown";
import type { DropDownItemProps } from "@/components/dropdown-item/DropDownItem.types";
import { TooltipIconButton } from "@/components/tooltip-icon-button";

const FILE_FORMATS = [".docx", ".pdf", ".xlsx"];

const ComposerActionAttachment = () => {
  const [isOpen, setIsOpen] = React.useState(false);

  const { addAttachmentFile } = useAttachmentsStore();

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
      [...FILE_FORMATS].join(" , "),
      true,
      (file) => {
        if (Array.isArray(file)) {
          file.forEach((file) => {
            const extension = file.split(".").pop() ?? "";

            if (FILE_FORMATS.includes(`.${extension}`)) {
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
                      extension === "pdf"
                        ? 87
                        : extension === "docx"
                        ? 65
                        : 257,
                  });
                }
              );
            }
          });
        }
      }
    );
  };

  const recentFiles = (
    JSON.parse(
      window.AscDesktopEditor?.callToolFunction("recent_files_reader") ?? "{}"
    ) as { files: { path: string; type: number }[] }
  )?.files
    ?.filter((file) => !file.path.includes("http"))
    .filter((file) => {
      const isDocument = file.type === 65;
      const isPDF = file.type === 87;
      const isSpreadsheet = file.type === 257;

      return isDocument || isPDF || isSpreadsheet;
    })
    ?.map((file) => {
      let icon = DocumentsIconSvg;

      if (file.type === 87) {
        icon = PdfIconSvg;
      } else if (file.type === 257) {
        icon = SpreadsheetsIconSvg;
      }

      return {
        text: file.path.split("/").pop() ?? "",
        key: file.path,
        id: file.path,
        icon: <ReactSVG src={icon} />,
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
