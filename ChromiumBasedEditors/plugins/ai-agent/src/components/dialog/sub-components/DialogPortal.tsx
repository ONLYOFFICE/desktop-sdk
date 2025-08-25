import React from "react";
import * as DialogPrimitive from "@radix-ui/react-dialog";

const DialogPortal = ({
  ...props
}: React.ComponentProps<typeof DialogPrimitive.Portal>) => {
  return <DialogPrimitive.Portal data-slot="dialog-portal" {...props} />;
};

export { DialogPortal };
