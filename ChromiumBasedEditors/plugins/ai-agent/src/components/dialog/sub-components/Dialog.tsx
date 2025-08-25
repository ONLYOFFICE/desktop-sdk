import React from "react";
import * as DialogPrimitive from "@radix-ui/react-dialog";

const Dialog = ({
  ...props
}: React.ComponentProps<typeof DialogPrimitive.Root>) => {
  return <DialogPrimitive.Root data-slot="dialog" {...props} />;
};

export { Dialog };
