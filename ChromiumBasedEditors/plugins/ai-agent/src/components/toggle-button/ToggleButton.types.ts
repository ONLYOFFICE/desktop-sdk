export type ToggleButtonProps = {
  checked: boolean;
  onCheckedChange: (checked: boolean) => void;
  disabled?: boolean;
  size?: "small" | "medium" | "large";
  label?: string;
  id?: string;
  className?: string;
};
