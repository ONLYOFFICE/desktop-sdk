import React from "react";
import * as DialogPrimitive from "@radix-ui/react-dialog";

import { cn } from "@/lib/utils";

const DialogTitle = ({
  className,
  withWarningIcon,
  ...props
}: React.ComponentProps<typeof DialogPrimitive.Title> & {
  withWarningIcon?: boolean;
}) => {
  return (
    <DialogPrimitive.Title
      data-slot="dialog-title"
      className={cn(
        "font-bold text-[var(--modal-dialog-header-color)] select-none",
        className,
        withWarningIcon
          ? "text-[14px] leading-[20px]"
          : "text-[18px] leading-[24px]"
      )}
      {...props}
    />
  );
};

export { DialogTitle };
