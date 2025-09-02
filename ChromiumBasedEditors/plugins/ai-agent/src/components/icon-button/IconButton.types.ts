import type { ButtonHTMLAttributes } from "react";

export type IconButtonProps = {
  iconName: string;
  size: number;
  color?: string;
  isStroke?: boolean;
} & ButtonHTMLAttributes<HTMLButtonElement>;
