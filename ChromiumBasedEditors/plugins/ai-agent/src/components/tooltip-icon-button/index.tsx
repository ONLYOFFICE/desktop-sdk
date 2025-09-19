"use client";

import { forwardRef } from "react";

import { cn } from "../../lib/utils";

import { Tooltip, TooltipContent, TooltipTrigger } from "../tooltip";

import type { TooltipIconButtonProps } from "./TooltipIconButton.types";

export const TooltipIconButton = forwardRef<
  HTMLDivElement,
  TooltipIconButtonProps
>(
  (
    { children, tooltip, side = "bottom", visible = true, className, ...rest },
    ref
  ) => {
    return (
      <Tooltip>
        <TooltipTrigger asChild>
          <div
            className={cn("flex items-center gap-2", className)}
            ref={ref}
            {...rest}
          >
            {children}
          </div>
        </TooltipTrigger>
        <TooltipContent className={visible ? "" : "hidden"} side={side}>
          {tooltip}
        </TooltipContent>
      </Tooltip>
    );
  }
);

TooltipIconButton.displayName = "TooltipIconButton";
