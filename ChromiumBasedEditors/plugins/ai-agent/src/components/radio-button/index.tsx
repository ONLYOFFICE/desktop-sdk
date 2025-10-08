import React from "react";
import type { RadioButtonProps } from "./RadioButton.types";

const RadioButton = ({
  checked = false,
  onChange,
  disabled = false,
  ...props
}: RadioButtonProps) => {
  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (!disabled && onChange) {
      onChange(event.target.checked);
    }
  };

  return (
    <label className="inline-flex items-center cursor-pointer">
      <input
        type="radio"
        checked={checked}
        onChange={handleChange}
        disabled={disabled}
        className="sr-only"
        {...props}
      />
      <div
        className={`
          w-[20px] h-[20px] rounded-full border-[1px] flex items-center justify-center transition-all
          ${
            checked
              ? "border-[var(--radio-button-active-color)]"
              : "border-[var(--radio-button-color)]"
          }
          ${disabled ? "opacity-50 cursor-not-allowed" : "cursor-pointer"}
        `}
      >
        {checked && (
          <div className="w-[12px] h-[12px] rounded-full bg-[var(--radio-button-active-color)]" />
        )}
      </div>
    </label>
  );
};

export { RadioButton };
