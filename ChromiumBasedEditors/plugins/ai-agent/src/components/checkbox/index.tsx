import React, { useCallback } from "react";
import { ReactSVG } from "react-svg";

import CheckedIconUrl from "@/assets/checked.svg?url";
import { cn } from "@/lib/utils";

type CheckboxProps = {
  checked?: boolean;
  onChange?: (checked: boolean) => void;
  disabled?: boolean;
  id?: string;
  className?: string;
};

const Checkbox = ({
  checked = false,
  onChange,
  disabled = false,
  id,
  className = "",
}: CheckboxProps) => {
  const handleChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    if (onChange && !disabled) {
      onChange(event.target.checked);
    }
  };

  const handleBeforeInjection = useCallback((svg: SVGSVGElement) => {
    const path = svg.querySelector("path");
    if (path) {
      path.setAttribute("stroke", "var(--checkbox-active-icon-color)");
    }
  }, []);

  // Container styles
  const containerStyles = "inline-flex items-center relative group";

  // Input styles
  const inputBaseStyles = "absolute opacity-0 cursor-pointer w-4 h-4 m-0";
  const inputDisabledStyles = "disabled:cursor-not-allowed";

  // Label styles
  const labelBaseStyles = "inline-flex items-center select-none";
  const labelCursorStyles = disabled
    ? "cursor-not-allowed opacity-60"
    : "cursor-pointer";

  // Checkbox box styles
  const boxBaseStyles =
    "w-4 h-4 rounded-[2px] flex items-center justify-center transition-all duration-200 box-border";
  const boxFocusStyles = "focus-within:outline-none";

  // Unchecked state styles
  const boxUncheckedBaseStyles =
    "bg-[var(--checkbox-bg-color)] border border-[var(--checkbox-color)]";
  const boxUncheckedHoverStyles = !disabled
    ? "group-hover:!bg-[var(--checkbox-bg-hover-color)] group-active:!bg-[var(--checkbox-bg-pressed-color)]"
    : "";

  // Checked state styles
  const boxCheckedBaseStyles = "bg-[var(--checkbox-active-color)]";
  const boxCheckedHoverStyles = !disabled
    ? "group-hover:!bg-[var(--checkbox-active-hover-color)] group-active:!bg-[var(--checkbox-active-pressed-color)]"
    : "";

  // Icon styles
  const iconStyles = "flex items-center justify-center";

  return (
    <div className={cn(containerStyles, className)}>
      <input
        type="checkbox"
        id={id}
        checked={checked}
        onChange={handleChange}
        disabled={disabled}
        className={cn(inputBaseStyles, inputDisabledStyles)}
      />
      <label htmlFor={id} className={cn(labelBaseStyles, labelCursorStyles)}>
        <div
          className={cn(
            boxBaseStyles,
            boxFocusStyles,
            checked
              ? cn(boxCheckedBaseStyles, boxCheckedHoverStyles)
              : cn(boxUncheckedBaseStyles, boxUncheckedHoverStyles)
          )}
        >
          {checked ? (
            <ReactSVG
              src={CheckedIconUrl}
              className={iconStyles}
              beforeInjection={handleBeforeInjection}
            />
          ) : null}
        </div>
      </label>
    </div>
  );
};

export { Checkbox };
