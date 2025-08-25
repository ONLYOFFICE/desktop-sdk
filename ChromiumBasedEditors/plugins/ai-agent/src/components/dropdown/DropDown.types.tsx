import type { DropDownItemProps } from "../dropdown-item/DropDownItem.types";

export type DropdownMenuProps = {
  trigger: React.ReactNode;
  items: DropDownItemProps[];
  side?: "bottom" | "top" | "left" | "right";
  align?: "start" | "center" | "end";
  contentClassName?: string;
  maxWidth?: number | string;
};
