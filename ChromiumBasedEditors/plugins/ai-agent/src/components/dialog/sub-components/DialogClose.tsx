import React from "react";
import * as DialogPrimitive from "@radix-ui/react-dialog";

const DialogClose = ({
  ...props
}: React.ComponentProps<typeof DialogPrimitive.Close>) => {
  return <DialogPrimitive.Close data-slot="dialog-close" {...props} />;
};

export { DialogClose };
