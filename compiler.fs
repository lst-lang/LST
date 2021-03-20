\ This file is part of LST.

\ LST is free software; you can redistribute it and/or modify
\ it under the terms of the GNU General Public License as
\ published by the Free Software Foundation, either version 3
\ of the License, or (at your option) any later version.

\ This program is distributed in the hope that it will be
\ useful, but WITHOUT ANY WARRANTY; without even the implied
\ warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
\ PURPOSE. See the GNU General Public License for more details.

\ You should have received a copy of the GNU General Public
\ License along with this program.
\ If not, see http://www.gnu.org/licenses/.

\ compiler            sexpr template                         lst
[ ' allocate ' allocate-template defer! ]
[ %any ] template refany-template
[ %pointer ] template refsexpr-template
[ %flexible ] template string-template
[ 4 cells nil-template array-element ]
[ 1 array-bounds 1 array-size ]
[ 0 1 0 !upper 0 1 0 !lower ]
[ %record ] template cons-template [ 2 more-fields ]
[ 0 nil-template 0 !field ]
[ 1 cells nil-template 1 !field ]
[ %variants ] template sexpr-template [ 5 more-fields ]
[ 0 nil-template 0 !field ]
[ 0 nil-template 1 !field ]
[ 0 string-template 2 !field ]
[ 0 refany-template 3 !field ]
[ 0 cons-template 4 !field ]
[ 5 cells refsexpr-template object-size ! ]
[ sexpr-template refsexpr-template object-template ! ]

\ compiler            runtime template                       lst
[ %record ] template runtime-template [ 1 more-fields ]
[ 0 refsexpr-template 0 !field ]
[ 1 cells +stack runtime-template +frame ]

[ hp @ heap - 1 cells / . cr ]
[ mark-root ]
[ compact ]
[ update-root ]
[ hp @ heap - 1 cells / . cr ]
