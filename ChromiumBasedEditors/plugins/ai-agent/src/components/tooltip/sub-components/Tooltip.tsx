import React from "react";
import * as TooltipPrimitive from "@radix-ui/react-tooltip";

import { TooltipProvider } from "./Provider";

const Tooltip = ({
  ...props
}: React.ComponentProps<typeof TooltipPrimitive.Root>) => {
  return (
    <TooltipProvider>
      <TooltipPrimitive.Root data-slot="tooltip" {...props} />
    </TooltipProvider>
  );
};

export { Tooltip };
