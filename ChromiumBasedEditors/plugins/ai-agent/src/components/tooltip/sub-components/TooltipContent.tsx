import React from "react";
import * as TooltipPrimitive from "@radix-ui/react-tooltip";

import { cn } from "@/lib/utils";

const TooltipContent = ({
  className,
  sideOffset = 0,
  children,
  ...props
}: React.ComponentProps<typeof TooltipPrimitive.Content>) => {
  return (
    <TooltipPrimitive.Portal container={document.getElementById("app")}>
      <TooltipPrimitive.Content
        data-slot="tooltip-content"
        sideOffset={sideOffset}
        className={cn(
          "mt-[4px] z-[9999]",
          "bg-[var(--tooltip-background-color)] text-[var(--tooltip-text-color)] border border-[var(--tooltip-border-color)] shadow-[var(--tooltip-shadow)] p-[4px] rounded-[4px] text-[12px] font-[400] leading-[16px]",
          className
        )}
        {...props}
      >
        {children}
      </TooltipPrimitive.Content>
    </TooltipPrimitive.Portal>
  );
};

export { TooltipContent };
