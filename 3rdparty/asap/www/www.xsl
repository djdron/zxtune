<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:param name="version" />
	<xsl:output method="html" />

	<xsl:template match="/page">
		<xsl:text disable-output-escaping="yes">&lt;!DOCTYPE html&gt;
</xsl:text>
		<html lang="en">
			<head>
				<title>
					<xsl:text>ASAP</xsl:text>
					<xsl:if test="@title != 'Home'">
						<xsl:text> - </xsl:text>
						<xsl:value-of select="@title" />
					</xsl:if>
				</title>
				<meta name="viewport" content="initial-scale=1" />
				<style>
					html { background-color: #fff; color: #000; font-family: Segoe UI,Helvetica,Arial,sans-serif; }
					img { max-width: 95%; height: auto; }
					h1, h2, h3, dt, nav li.tab_selected { color: #c22; }
					a, .menuToggle:hover, nav li a:hover { color: #00c; }
					nav ul { padding: 0px; }
					nav li { display: inline; }
					nav li a { padding: 0.5em; color: #999; text-decoration: none; }
					nav li.tab_selected { padding: 0.5em; }
					@media (max-width: 990px) {
						.menuToggle { float: right; background-color: #c22; color: #fff; border: none; font-size: 2.5em; }
						.collapsed { display: none; }
						nav li { display: block; padding: 0.5em 0px; text-align: right; }
					}
					@media (min-width: 991px) {
						html { padding: 0em 3em; }
						.menuToggle { display: none; }
					}
					dt { margin-top: 1em; }
					.author { color: #c22; padding-right: 1em; text-align: right; }
					.rip { border: solid #000 1px; padding-left: 2px; padding-right: 2px; color: #000; }
					pre { background-color: #eee; padding: 1ex; }
				</style>
				<script>
					function toggleMenu()
					{
						const menu = document.getElementById("menu");
						menu.className = menu.className == "collapsed" ? "" : "collapsed";
					}
				</script>
				<xsl:apply-templates select="script" />
			</head>
			<body>
				<input type="button" class="menuToggle" value="&#8801;" onclick="toggleMenu()" />
				<header>
					<h1>ASAP - Another Slight Atari Player</h1>
				</header>
				<nav>
					<ul id="menu" class="collapsed">
						<xsl:call-template name="menu"><xsl:with-param name="page">Home</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Android</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Windows</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">macOS</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Linux</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Web</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Formats</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Convert</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">News</xsl:with-param></xsl:call-template>
						<xsl:call-template name="menu"><xsl:with-param name="page">Contact</xsl:with-param></xsl:call-template>
					</ul>
				</nav>
				<main>
					<xsl:apply-templates select="*[not(self::script)]" />
				</main>
				<footer>
					<p>
						<a href="https://sourceforge.net/p/asap/">
							<img alt="Download ASAP" src="https://sourceforge.net/sflogo.php?type=13&amp;group_id=154391" />
						</a>
					</p>
				</footer>
			</body>
		</html>
	</xsl:template>

	<xsl:template name="menu">
		<xsl:param name="page" />
		<li>
			<xsl:choose>
				<xsl:when test="$page = /page/@title">
					<xsl:attribute name="class">tab_selected</xsl:attribute>
					<xsl:value-of select="$page" />
				</xsl:when>
				<xsl:when test="$page = 'Home'">
					<a href="/">Home</a>
				</xsl:when>
				<xsl:otherwise>
					<a href="{translate($page, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')}.html">
						<xsl:value-of select="$page" />
					</a>
				</xsl:otherwise>
			</xsl:choose>
		</li>
	</xsl:template>

	<xsl:template match="version">
		<xsl:value-of select="$version" />
	</xsl:template>

	<xsl:template match="release">
		<h2>
			<xsl:text>ASAP&#160;</xsl:text>
			<xsl:value-of select="@version" />
			<xsl:text>&#160;(</xsl:text>
			<xsl:value-of select="@date" />
			<xsl:text>)</xsl:text>
		</h2>
		<xsl:apply-templates />
	</xsl:template>

	<xsl:template match="download">
		<xsl:variable name="file" select="concat(@prefix, $version, @suffix)" />
		<a href="https://sourceforge.net/projects/asap/files/asap/{$version}/{$file}/download"><xsl:value-of select="$file" /></a>
	</xsl:template>

	<xsl:template match="authors">
		<h2>Authors</h2>
		<table>
			<xsl:for-each select="author">
				<tr>
					<td class="author">
						<xsl:choose>
							<xsl:when test="@rip">
								<span class="rip"><xsl:value-of select="@name" /></span>
							</xsl:when>
							<xsl:when test="@href">
								<a href="{@href}"><xsl:value-of select="@name" /></a>
							</xsl:when>
							<xsl:otherwise>
								<xsl:value-of select="@name" />
							</xsl:otherwise>
						</xsl:choose>
					</td>
					<td>
						<xsl:apply-templates />
					</td>
				</tr>
			</xsl:for-each>
		</table>
	</xsl:template>

	<xsl:template match="ul[@title]">
		<p><xsl:value-of select="@title" />:</p>
		<ul>
			<xsl:apply-templates />
		</ul>
	</xsl:template>

	<xsl:template match="a[@href]|code|div|dd|dl|dt|em|h2|h3|img|input|li|ol|p|pre|script|select|ul">
		<xsl:element name="{name()}">
			<xsl:copy-of select="@*" />
			<xsl:apply-templates />
		</xsl:element>
	</xsl:template>

	<xsl:template match="*" />
</xsl:stylesheet>
