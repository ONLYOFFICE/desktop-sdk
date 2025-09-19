import React from "react";
import * as AvatarPrimitive from "@radix-ui/react-avatar";

import { cn } from "../../lib/utils";

const AvatarImage = ({
  className,
  ...props
}: React.ComponentProps<typeof AvatarPrimitive.Image>) => {
  return (
    <AvatarPrimitive.Image
      data-slot="avatar-image"
      className={cn("aspect-square size-full", className)}
      {...props}
    />
  );
};

export { AvatarImage };
