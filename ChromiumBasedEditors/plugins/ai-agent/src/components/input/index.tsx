import React, { useCallback } from "react";
import { ReactSVG } from "react-svg";

import ClearSearchIconUrl from "@/assets/clear.search.svg?url";

import { cn } from "@/lib/utils";

type InputProps = React.InputHTMLAttributes<HTMLInputElement> & {
  isError?: boolean;
  icon?: string;
  onClear?: () => void;
};

const Input = React.forwardRef<HTMLInputElement, InputProps>(
  ({ className, isError, icon, onClear, ...props }, ref) => {
    const handleBeforeInjection = useCallback((svg: SVGSVGElement) => {
      const paths = svg.querySelectorAll("path");
      paths.forEach((path) => {
        path.setAttribute("stroke", "var(--input-color)");
      });
      const circles = svg.querySelectorAll("circle");
      circles.forEach((circle) => {
        circle.setAttribute("fill", "var(--input-color)");
      });
    }, []);

    return (
      <div className={`relative ${className}`}>
        {icon && (
          <ReactSVG
            className="absolute left-[10px] top-[50%] translate-y-[-50%] w-[20px] h-[20px] flex items-center justify-center"
            src={icon}
            beforeInjection={handleBeforeInjection}
          />
        )}
        <input
          ref={ref}
          type={props.type ?? "text"}
          className={cn(
            "h-[32px] rounded-[4px] box-border border border-[var(--input-border-color)]",
            "bg-[var(--input-background-color)]",
            props.disabled
              ? ""
              : "hover:bg-[var(--input-hover-background-color)] hover:border-[var(--input-hover-border-color)]",
            "focus:bg-[var(--input-active-background-color)] focus:border focus:border-[var(--input-active-border-color)]",
            "outline-none",
            "placeholder:text-[var(--input-placeholder-color)] text-[var(--input-color)]",
            "[&::-webkit-search-cancel-button]:hidden [&::-webkit-search-decoration]:hidden",
            icon ? "ps-[40px]" : "ps-[12px]",
            props.type === "search" && props.value ? "pe-[40px]" : "pe-[2px]",
            className
          )}
          style={{
            border: isError ? "1px solid var(--input-error-color)" : "",
          }}
          value={props.value}
          {...props}
        ></input>
        {props.type === "search" && props.value && (
          <button
            type="button"
            onClick={onClear}
            className="absolute right-[10px] top-[50%] translate-y-[-50%] w-[20px] h-[20px] flex items-center justify-center cursor-pointer"
          >
            <ReactSVG
              src={ClearSearchIconUrl}
              beforeInjection={handleBeforeInjection}
            />
          </button>
        )}
      </div>
    );
  }
);

Input.displayName = "Input";

export { Input };
