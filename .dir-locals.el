;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil . ((eval . (setq flycheck-clang-include-path
		       (list "/usr/include/freetype2" "/usr/include/AL")))))
 (c++-mode . ((flycheck-clang-language-standard . "c++20")
	      (flycheck-gcc-language-standard . "c++20"))))
