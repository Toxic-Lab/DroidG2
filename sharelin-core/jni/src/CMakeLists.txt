link_directories ( ${Boost_LIBRARY_DIRS} )

set (SHARELIN_SRC
		system
		trafmanager
		history
		udp
		contenttype
		sourcehost
		arrivalcache
		departurecache
		g2nodeinfo
		g2packet
		g2misc
		g2handshake
		security
		hubcache
		updateswatch
		uicomparers
		downloadrequest
		logger
		settings
		filesystem
		searchmanager
		searcher	
		media
		searchdefs
		sourcehost
		asynctaskserver
		asynctask
		asynchash
		asynccopy
		sharesync
		sharedefs
		sharecache
		sharemanager
		managedinfo
		queryhashtable
		asyncfile
		sharedfile
		filemanager
		filemap
		uicommons
		downloadmanager
		uploadmanager
		download
		downloader
		downloader_impl
		uploader_impl
		discovery
		g2net
		g2node
		main
		gatekeeper
		transceiver
		webuiserver
		webuiclient
		webuicontrol
		io
    )

set(COMMON_SRC
		common/range
		common/segment
		common/ipsegment
		common/filerange
		common/buffer
		common/types
		common/staticname
		common/debug
		common/ip
		common/guid
		common/env
		common/zlib
		common/period
		common/url
		common/increment
		common/idgen
		common/foreach
		common/fio
		common/filemapptr
		common/tigertree
		common/hash/ed2khash
		common/hash/hash
		common/hash/sha1
		common/hash/ttr
		common/hash/md5
		common/hash/sboxes
		common/hash/tiger
		common/conv/hex
		common/conv/conv
		common/conv/base32
		common/conv/common
)

set (G2PACKET_SRC
		g2packet/util
		g2packet/packet
		g2packet/packettype
		g2packet/reader
		g2packet/writer
		g2packet/header
    )

set (UI_SRC
		ui/command
		ui/tag
		ui/template
		ui/sortprefs
    )


add_executable (sharelin ${SHARELIN_SRC} ${COMMON_SRC} ${G2PACKET_SRC} ${UI_SRC})

target_link_libraries ( sharelin 
		${Boost_THREAD_LIBRARY} 
		${Boost_IOSTREAMS_LIBRARY} 
		${Boost_FILESYSTEM_LIBRARY} 
		${Boost_SYSTEM_LIBRARY} 
		${ZLIB_LIBRARIES}
		)

