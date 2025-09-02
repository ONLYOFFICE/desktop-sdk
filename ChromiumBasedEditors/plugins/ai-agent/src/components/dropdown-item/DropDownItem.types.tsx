import type React from "react";

export type DropDownItemProps = {
  text: string;
  onClick: () => void;

  icon?: string | React.ReactNode;
  iconSize?: number;
  iconColor?: string;

  id?: string;
  isActive?: boolean;
  isSeparator?: boolean;

  withToggle?: boolean;
  toggleChecked?: boolean;
  onToggleChange?: (checked: boolean) => void;

  actionsMenu?: DropDownItemProps[];
  subMenu?: DropDownItemProps[];
};
