import type React from "react";
import type { DropDownItemProps } from "../dropdown-item/DropDownItem.types";

export type DropdownMenuProps = {
  trigger: React.ReactNode;
  items: DropDownItemProps[];
  side?: "bottom" | "top" | "left" | "right";
  align?: "start" | "center" | "end";
  sideOffset?: number;
  contentClassName?: string;
  maxWidth?: number | string;
  matchTriggerWidth?: boolean;
  open?: boolean;
  containerRef?: HTMLDivElement | null;
  dropdownRef?: React.RefObject<HTMLDivElement | null>;
  onOpenChange?: (open: boolean) => void;
};
