import React from "react";
import { ReactSVG } from "react-svg";

import { cn } from "@/lib/utils";

type InputProps = React.InputHTMLAttributes<HTMLInputElement> & {
  isError?: boolean;
  icon?: string;
  iconColor?: string;
};

const Input = React.forwardRef<HTMLInputElement, InputProps>(
  ({ className, isError, icon, iconColor, ...props }, ref) => {
    return (
      <div className={`relative ${className}`}>
        {icon && (
          <ReactSVG
            className="absolute left-[10px] top-[50%] translate-y-[-50%] w-[20px] h-[20px] flex items-center justify-center"
            src={icon}
            beforeInjection={(svg) => {
              const path = svg.querySelector("path");
              if (path) {
                path.setAttribute("stroke", iconColor || "var(--input-color)");
              }
            }}
          />
        )}
        <input
          ref={ref}
          type={props.type ?? "text"}
          className={cn(
            "h-[32px] rounded-[4px] pe-[2px] box-border border border-[var(--input-border-color)]",
            "bg-[var(--input-background-color)]",
            props.disabled
              ? ""
              : "hover:bg-[var(--input-hover-background-color)] hover:border-[var(--input-hover-border-color)]",
            "focus:bg-[var(--input-active-background-color)] focus:border focus:border-[var(--input-active-border-color)]",
            "outline-none",
            "placeholder:text-[var(--input-placeholder-color)] text-[var(--input-color)]",
            icon ? "ps-[40px]" : "ps-[12px]",
            className
          )}
          style={{
            border: isError ? "1px solid var(--input-error-color)" : "",
          }}
          value={props.value}
          {...props}
        ></input>
      </div>
    );
  }
);

Input.displayName = "Input";

export { Input };
