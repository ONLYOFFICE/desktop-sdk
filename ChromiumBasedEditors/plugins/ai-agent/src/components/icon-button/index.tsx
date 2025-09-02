import { ReactSVG } from "react-svg";

import type { IconButtonProps } from "./IconButton.types";

const IconButton = ({
  iconName,
  size,
  color,
  isStroke,
  ...props
}: IconButtonProps) => {
  return (
    <button
      className="border-none bg-none p-0 m-0 cursor-pointer flex items-center justify-center outline-none focus:outline-none focus-visible:outline-none"
      style={{
        width: `${size}px`,
        height: `${size}px`,
      }}
      {...props}
    >
      <ReactSVG
        src={iconName}
        className="flex items-center justify-center"
        beforeInjection={(svg) => {
          if (color) {
            const paths = svg.querySelectorAll("path");
            paths.forEach((path) => {
              if (!isStroke) {
                path.setAttribute("fill", color);
              } else {
                path.setAttribute("stroke", color);
              }
            });
          }
        }}
      />
    </button>
  );
};

export { IconButton };
