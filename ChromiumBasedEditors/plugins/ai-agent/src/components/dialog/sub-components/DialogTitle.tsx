import React from "react";
import * as DialogPrimitive from "@radix-ui/react-dialog";

import { cn } from "../../../lib/utils";

const DialogTitle = ({
  className,
  ...props
}: React.ComponentProps<typeof DialogPrimitive.Title>) => {
  return (
    <DialogPrimitive.Title
      data-slot="dialog-title"
      className={cn("text-lg leading-none font-semibold", className)}
      {...props}
    />
  );
};

export { DialogTitle };
