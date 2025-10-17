import { ReactSVG } from "react-svg";

import type { TAttachmentFile } from "@/lib/types";
import { cn } from "@/lib/utils";

import useAttachmentsStore from "@/store/useAttachmentsStore";

import BtnCloseIconUrl from "@/assets/btn-close.small.svg?url";
import DocumentsIconUrl from "@/assets/formats/32/documents.svg?url";
import PdfIconUrl from "@/assets/formats/32/pdf.svg?url";
import SpreadsheetsIconUrl from "@/assets/formats/32//spreadsheets.svg?url";

import { IconButton } from "../icon-button";

type FileItemProps = {
  file: TAttachmentFile;
  withoutClose?: boolean;
};

const FileItem = ({ file, withoutClose }: FileItemProps) => {
  const { deleteAttachmentFile } = useAttachmentsStore();

  const handleDelete = () => {
    deleteAttachmentFile(file.path);
  };

  const name = file.path.split("/").pop() ?? "";
  const extension = name.split(".").pop() ?? "";
  const nameWithoutExtension = name.replace(`.${extension}`, "");

  const icon =
    extension === "pdf"
      ? PdfIconUrl
      : extension === "docx"
      ? DocumentsIconUrl
      : SpreadsheetsIconUrl;

  return (
    <div
      className={cn(
        " w-fit flex flex-row gap-[12px] h-[44px] rounded-[8px] p-[4px] box-border border-[var(--file-items-border-color)]",
        withoutClose ? "cursor-pointer pe-[24px]" : "",
        withoutClose
          ? "bg-[var(--file-items-chat-background-color)]"
          : "border bg-[var(--file-items-background-color)]",
        withoutClose
          ? "hover:bg-[var(--file-items-chat-hover-background-color)]"
          : "",
        withoutClose
          ? "active:bg-[var(--file-items-chat-pressed-background-color)]"
          : ""
      )}
      onClick={() => {
        if (!withoutClose) return;

        window.AscDesktopEditor.openTemplate(file.path, name);
      }}
    >
      {file.isImage ? (
        <img className="h-[31px]" src={file.content} alt="" />
      ) : (
        <div className="flex flex-row items-center h-[36px] gap-[4px]">
          <ReactSVG className="" src={icon} />
          <p className="text-[var(--file-items-color)] whitespace-nowrap overflow-hidden text-ellipsis">
            {nameWithoutExtension}
            <span className="text-[var(--file-items-ext-color)]">
              .{extension}
            </span>
          </p>
        </div>
      )}

      {!withoutClose ? (
        <IconButton
          iconName={BtnCloseIconUrl}
          size={16}
          onClick={handleDelete}
        />
      ) : null}
    </div>
  );
};

export { FileItem };
