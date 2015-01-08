file(
	GLOB
	headers_files
	*.h 
)

# Installation targets
install (FILES ${headers_files} 
	 DESTINATION ${INCLUDE_INSTALL_DIR}
	 COMPONENT devel)

