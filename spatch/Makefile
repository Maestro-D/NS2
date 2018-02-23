# Project's name
NAME			=	spatch

# Shell commands
MAKE			+=	VERBOSE=1
RM			=	rm -rfv

# Local directories
BUILD_DIR		=	build
PACKAGING_DIR		=	packaging
CONFIG_DIR		=	config
SPATCH_CONFIG_DIR	=	${CONFIG_DIR}/spatch
SYSTEMD_CONFIG_DIR	=	${CONFIG_DIR}/systemd

# Compilation & packaging virtual environment
SSH_PORT		=	55022
SSH_USER		=	mr-spatch
SSH_PASSWORD		=	${SSH_USER}
SSH_HOSTNAME		=	localhost
SSH_USER_AT_HOST	=	${SSH_USER}@${SSH_HOSTNAME}
REMOTE_SPATCH_PATH	=	/home/${SSH_USER}/spatch
SSH_SPATCH_LOCATION	=	${SSH_USER_AT_HOST}:${REMOTE_SPATCH_PATH}
# Shell commands shortcuts
SSH			=	ssh -p ${SSH_PORT}
SCP			=	scp -P ${SSH_PORT}

# Makefile rules
all:		${NAME}

${NAME}:	build

# Spatch compilation
build:
	@echo "Building spatch from source..."
	mkdir -p ${BUILD_DIR}
	cd ${BUILD_DIR} && \
	cmake .. && \
	${MAKE}

package:	local_package

# Build debian package on local host
local_package:
	@echo "Packaging spatch for debian 64bits..."
	cd ${PACKAGING_DIR} && \
	./build.sh

# Build debian package inside virtual environment
remote_package:	fclean_package
	@echo "Packaging spatch for debian 64bits inside virtual environment..."
	${SSH} ${SSH_USER_AT_HOST} -t "echo 'ssh connection test'"
	${SSH} ${SSH_USER_AT_HOST} -t "${RM} ${REMOTE_SPATCH_PATH}"
	${SCP} -r . ${SSH_SPATCH_LOCATION} || true
	${SSH} ${SSH_USER_AT_HOST} -t "cd ${REMOTE_SPATCH_PATH} && make re local_package"
	${SCP} -r ${SSH_SPATCH_LOCATION}/${PACKAGING_DIR}/${NAME}_* ${PACKAGING_DIR}
	@echo
	@echo "Package is '`pwd`/`ls ${PACKAGING_DIR}/*.deb`'"

# Install and enable package in virtual environment
deployment:
	@echo "Deploying package inside testing environment..."
	${SSH} ${SSH_USER_AT_HOST} -t "echo 'ssh connection test'"
	${SSH} ${SSH_USER_AT_HOST} -t "echo ${SSH_PASSWORD} | sudo su -c \" \
	cd ${REMOTE_SPATCH_PATH}/${PACKAGING_DIR} && ls ${NAME}\"*\".deb 1>/dev/null && \
	systemctl stop ${NAME}.service ; \
	systemctl status ${NAME}.service ; echo ; \
	dpkg -i ${NAME}_\"*\".deb ; echo ; \
	systemctl start ${NAME}.service ; \
	sleep 3 ; \
	systemctl status ${NAME}.service \
	\""

clean:		clean_build

# Clean build directory
clean_build:
	@echo "Soft cleaning of build directory..."
	if [ -f ${BUILD_DIR}/Makefile ]; then \
		cd ${BUILD_DIR} && \
		${MAKE} clean; \
	fi

fclean:		fclean_build

# Delete build directory
fclean_build:
	@echo "Hard cleaning of build directory..."
	${RM} ${BUILD_DIR}

# Delete built package
fclean_package:
	@echo "Hard cleaning of packaging directory..."
	${RM} ${PACKAGING_DIR}/${NAME}_*

re:		fclean all

.PHONY:		all ${NAME} clean fclean re \
		build clean_build fclean_build \
		package local_package fclean_package \
		remote_package \
		deployment
