import type { ComponentPropsWithoutRef } from "react";

import type { Button } from "../button";

export type TooltipIconButtonProps = ComponentPropsWithoutRef<typeof Button> & {
  tooltip: string;
  side?: "top" | "bottom" | "left" | "right";
};
