import type { PropsWithChildren } from "react";

import { DialogContent as DialogPrimitiveContent } from "@radix-ui/react-dialog";

import {
  Dialog,
  DialogTrigger,
  DialogPortal,
  DialogOverlay,
  DialogTitle,
} from "../../dialog";

import useAttachmentSrc from "../hooks/useAttachmentSrc";

import AttachmentPreview from "./Preview";

const AttachmentPreviewDialog = ({ children }: PropsWithChildren) => {
  const src = useAttachmentSrc();

  if (!src) return children;

  return (
    <Dialog>
      <DialogTrigger
        className="hover:bg-accent/50 cursor-pointer transition-colors"
        asChild
      >
        {children}
      </DialogTrigger>
      <DialogPortal>
        <DialogOverlay />
        <DialogPrimitiveContent className="data-[state=open]:animate-in data-[state=closed]:animate-out data-[state=closed]:fade-out-0 data-[state=open]:fade-in-0 data-[state=closed]:zoom-out-95 data-[state=open]:zoom-in-95 data-[state=closed]:slide-out-to-left-1/2 data-[state=closed]:slide-out-to-top-[48%] data-[state=open]:slide-in-from-left-1/2 data-[state=open]:slide-in-from-top-[48%] fixed left-[50%] top-[50%] z-50 grid translate-x-[-50%] translate-y-[-50%] shadow-lg duration-200">
          <DialogTitle className="sr-only">
            Image Attachment Preview
          </DialogTitle>
          <AttachmentPreview src={src} />
        </DialogPrimitiveContent>
      </DialogPortal>
    </Dialog>
  );
};

export default AttachmentPreviewDialog;
