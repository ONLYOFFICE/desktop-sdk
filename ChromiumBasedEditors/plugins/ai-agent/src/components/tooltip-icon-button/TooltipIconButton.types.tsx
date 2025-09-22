import type { ComponentPropsWithoutRef } from "react";

export type TooltipIconButtonProps = ComponentPropsWithoutRef<"div"> & {
  tooltip: string;
  side?: "top" | "bottom" | "left" | "right";
  visible?: boolean;
};
