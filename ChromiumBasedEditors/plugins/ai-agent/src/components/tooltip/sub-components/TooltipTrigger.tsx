import React from "react";
import * as TooltipPrimitive from "@radix-ui/react-tooltip";

const TooltipTrigger = ({
  ...props
}: React.ComponentProps<typeof TooltipPrimitive.Trigger>) => {
  return <TooltipPrimitive.Trigger data-slot="tooltip-trigger" {...props} />;
};

export { TooltipTrigger };
