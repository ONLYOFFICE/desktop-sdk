import type React from "react";

export type DropDownItemProps = {
  text: string;
  onClick: (e: Event) => void;

  icon?: string | React.ReactNode;
  iconSize?: number;
  iconColor?: string;

  id?: string;
  isActive?: boolean;
  isSeparator?: boolean;

  withToggle?: boolean;
  toggleChecked?: boolean;
  onToggleChange?: (checked: boolean) => void;
  toggleDisabled?: boolean;

  subMenu?: DropDownItemProps[];

  checked?: boolean;
};
