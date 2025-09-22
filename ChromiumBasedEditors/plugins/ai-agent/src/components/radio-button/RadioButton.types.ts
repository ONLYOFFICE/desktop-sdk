import type { InputHTMLAttributes } from "react";

export type RadioButtonProps = {
  checked?: boolean;
  onChange?: (checked: boolean) => void;
  disabled?: boolean;
} & Omit<InputHTMLAttributes<HTMLInputElement>, "type" | "checked" | "onChange">;
