import type { ButtonHTMLAttributes } from "react";

export type IconButtonProps = {
  iconName: string;
  size: number;
  color?: string;
  isStroke?: boolean;
  isTransform?: boolean;
  isActive?: boolean;
  insideElement?: boolean;
} & ButtonHTMLAttributes<HTMLButtonElement>;
