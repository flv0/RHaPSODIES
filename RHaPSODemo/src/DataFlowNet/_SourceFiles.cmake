# $Id$

set( RelativeDir "src/DataFlowNet" )
set( RelativeSourceGroup "source\\DataFlowNet" )

set( DirFiles
	HandTrackingNode.cpp
	HandModelTestingNode.cpp
	_SourceFiles.cmake
)
set( DirFiles_SourceGroup "${RelativeSourceGroup}" )

set( LocalSourceGroupFiles  )
foreach( File ${DirFiles} )
	list( APPEND LocalSourceGroupFiles "${RelativeDir}/${File}" )
	list( APPEND ProjectSources "${RelativeDir}/${File}" )
endforeach()
source_group( ${DirFiles_SourceGroup} FILES ${LocalSourceGroupFiles} )
