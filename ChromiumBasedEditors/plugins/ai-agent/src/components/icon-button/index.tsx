import { ReactSVG } from "react-svg";

import { cn } from "@/lib/utils";

import type { IconButtonProps } from "./IconButton.types";

const IconButton = ({
  iconName,
  size,
  color,
  isStroke,
  isTransform,
  isActive,
  className,
  insideElement,
  disableHover,
  ...props
}: IconButtonProps) => {
  return (
    <button
      className={cn(
        `border-none cursor-pointer rounded-[4px] cursor-pointer bg-none p-0 m-0 `,
        `flex items-center justify-center `,
        !disableHover
          ? insideElement
            ? "hover:enabled:bg-[var(--icon-button-hover-on-active-background-color)]"
            : `hover:enabled:bg-[var(--icon-button-hover-background-color)]`
          : undefined,
        `active:enabled:bg-[var(--icon-button-pressed-background-color)]`,
        `disabled:cursor-not-allowed disabled:opacity-[0.5]`,
        `outline-none focus:outline-none focus-visible:outline-none`,
        `${className}`
      )}
      style={{
        width: `${size}px`,
        height: `${size}px`,
        ...(isActive && {
          backgroundColor: "var(--icon-button-pressed-background-color)",
        }),
      }}
      {...props}
    >
      <ReactSVG
        src={iconName}
        className={`flex items-center justify-center ${
          isTransform ? "rotate-90" : ""
        }`}
        beforeInjection={(svg) => {
          const paths = svg.querySelectorAll("path");
          paths.forEach((path) => {
            if (!isStroke) {
              path.setAttribute("fill", color ?? "var(--icon-button-color)");
            } else {
              path.setAttribute("stroke", color ?? "var(--icon-button-color)");
            }
          });
        }}
      />
    </button>
  );
};

export { IconButton };
