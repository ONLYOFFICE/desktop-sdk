import React from "react";
import { ReactSVG } from "react-svg";

import CheckedIconUrl from "@/assets/checked.svg?url";

interface CheckboxProps {
  checked?: boolean;
  onChange?: (checked: boolean) => void;
  disabled?: boolean;
  id?: string;
  className?: string;
  "aria-label"?: string;
}

const Checkbox = ({
  checked = false,
  onChange,
  disabled = false,
  id,
  className = "",
  "aria-label": ariaLabel,
}: CheckboxProps) => {
  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (onChange && !disabled) {
      onChange(event.target.checked);
    }
  };

  return (
    <div className={`inline-flex items-center relative ${className}`}>
      <input
        type="checkbox"
        id={id}
        checked={checked}
        onChange={handleChange}
        disabled={disabled}
        className="absolute opacity-0 cursor-pointer w-4 h-4 m-0 disabled:cursor-not-allowed"
        aria-label={ariaLabel}
      />
      <label
        htmlFor={id}
        className="inline-flex items-center cursor-pointer select-none disabled:cursor-not-allowed disabled:opacity-60"
      >
        <div
          className={`w-4 h-4 rounded-[2px] flex items-center justify-center transition-all duration-200 box-border ${
            checked
              ? "bg-[var(--checkbox-active-color)]"
              : "bg-transparent border border-[var(--checkbox-color)]"
          } focus-within:outline-none`}
        >
          {checked && (
            <ReactSVG
              src={CheckedIconUrl}
              className="flex items-center justify-center"
              beforeInjection={(svg) => {
                const path = svg.querySelector("path");
                if (path) {
                  path.setAttribute("stroke", "var(--сheckbox-active-icon-color)");
                }
              }}
            />
          )}
        </div>
      </label>
    </div>
  );
};

export { Checkbox };
