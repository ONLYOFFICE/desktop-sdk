import * as Switch from "@radix-ui/react-switch";

import { cn } from "@/lib/utils";

import type { ToggleButtonProps } from "./ToggleButton.types";

const ToggleButton = ({
  checked,
  onCheckedChange,
  disabled = false,
  size = "small",
  label,
  id,
  className,
}: ToggleButtonProps) => {
  const sizeClasses = {
    small: "w-[24px] h-[14px]",
    medium: "w-[42px] h-[24px]",
    large: "w-[52px] h-[30px]",
  };

  const thumbSizeClasses = {
    small:
      "w-[10px] h-[10px] translate-x-[1px] data-[state=checked]:translate-x-[10px]",
    medium:
      "w-[20px] h-[20px] translate-x-[2px] data-[state=checked]:translate-x-[20px]",
    large:
      "w-[26px] h-[26px] translate-x-[2px] data-[state=checked]:translate-x-[24px]",
  };

  return (
    <div className="flex items-center gap-2">
      <Switch.Root
        id={id}
        checked={checked}
        onCheckedChange={onCheckedChange}
        disabled={disabled}
        className={cn(
          "relative inline-flex align-center shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 ease-in-out",
          "focus:outline-none",
          "disabled:cursor-not-allowed disabled:opacity-50",
          "bg-[var(--toggle-button-off-backround-color)] data-[state=checked]:bg-[var(--toggle-button-backround-color)]",
          sizeClasses[size],
          className
        )}
      >
        <Switch.Thumb
          className={cn(
            "pointer-events-none inline-block rounded-full bg-[var(--toggle-button-circle-color)] ring-0 transition-transform duration-200 ease-in-out",
            thumbSizeClasses[size]
          )}
        />
      </Switch.Root>
      {label && (
        <label
          htmlFor={id}
          className="text-sm font-medium text-gray-700 cursor-pointer"
        >
          {label}
        </label>
      )}
    </div>
  );
};

export { ToggleButton };
