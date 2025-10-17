import { useCallback } from "react";
import { ReactSVG } from "react-svg";

import { cn } from "@/lib/utils";

import type { IconButtonProps } from "./IconButton.types";

const IconButton = ({
  iconName,
  size,
  isStroke,
  isTransform,
  isActive,
  className,
  insideElement,
  disableHover,
  color,
  ...props
}: IconButtonProps) => {
  const hoverStyles = !disableHover
    ? insideElement
      ? "hover:enabled:bg-[var(--icon-button-hover-on-active-background-color)]"
      : "hover:enabled:bg-[var(--icon-button-hover-background-color)]"
    : undefined;

  const activeStyles = !disableHover
    ? "active:enabled:bg-[var(--icon-button-pressed-background-color)]"
    : undefined;

  const baseStyles = "border-none cursor-pointer rounded-[4px] bg-none p-0 m-0";
  const layoutStyles = "flex items-center justify-center";
  const disabledStyles = "disabled:cursor-not-allowed disabled:opacity-[0.5]";
  const focusStyles =
    "outline-none focus:outline-none focus-visible:outline-none";

  const handleBeforeInjection = useCallback(
    (svg: SVGSVGElement) => {
      const paths = svg.querySelectorAll("path");
      paths.forEach((path) => {
        if (!isStroke) {
          path.setAttribute("fill", color || "var(--icon-button-color)");
        } else {
          path.setAttribute("stroke", color || "var(--icon-button-color)");
        }
      });
    },
    [isStroke, color]
  );

  return (
    <button
      className={cn(
        baseStyles,
        layoutStyles,
        hoverStyles,
        activeStyles,
        disabledStyles,
        focusStyles,
        className
      )}
      style={{
        width: `${size}px`,
        height: `${size}px`,
        ...(isActive
          ? {
              backgroundColor: "var(--icon-button-pressed-background-color)",
            }
          : {}),
      }}
      {...props}
    >
      <ReactSVG
        src={iconName}
        className={`flex items-center justify-center ${
          isTransform ? "rotate-90" : ""
        }`}
        beforeInjection={handleBeforeInjection}
      />
    </button>
  );
};

export { IconButton };
