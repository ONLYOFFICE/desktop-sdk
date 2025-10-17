import * as Switch from "@radix-ui/react-switch";

import { cn } from "@/lib/utils";

import type { ToggleButtonProps } from "./ToggleButton.types";

const ToggleButton = ({
  checked,
  onCheckedChange,
  disabled = false,
  id,
  className,
}: ToggleButtonProps) => {
  // Base styles
  const baseStyles =
    "relative inline-flex align-center shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 ease-in-out";
  const focusStyles = "focus:outline-none";
  const disabledStyles = "disabled:cursor-not-allowed disabled:opacity-50";

  // State styles
  const backgroundStyles =
    "bg-[var(--toggle-button-off-backround-color)] data-[state=checked]:bg-[var(--toggle-button-backround-color)]";
  const hoverStyles =
    "hover:enabled:bg-[var(--toggle-button-off-background-hover-color)] data-[state=checked]:hover:enabled:bg-[var(--toggle-button-backround-hover-color)]";

  // Size styles
  const sizeStyles = "w-[24px] h-[14px]";

  // Thumb styles
  const thumbBaseStyles =
    "pointer-events-none inline-block rounded-full bg-[var(--toggle-button-circle-color)] ring-0 transition-transform duration-200 ease-in-out";
  const thumbSizeStyles =
    "w-[10px] h-[10px] translate-x-[1px] data-[state=checked]:translate-x-[10px]";

  return (
    <div className="flex items-center gap-2">
      <Switch.Root
        id={id}
        checked={checked}
        onCheckedChange={onCheckedChange}
        disabled={disabled}
        className={cn(
          baseStyles,
          focusStyles,
          disabledStyles,
          backgroundStyles,
          hoverStyles,
          sizeStyles,
          className
        )}
      >
        <Switch.Thumb className={cn(thumbBaseStyles, thumbSizeStyles)} />
      </Switch.Root>
    </div>
  );
};

export { ToggleButton };
