<?xml version="1.0"?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>
<xsl:template match="/message">

<!--line between messages -->
<hr/>

<!-- message header :: BEGIN -->
<table cellspacing="0" cellpadding="3" border="0" width="100%" align="top">

<!-- background color of incoming/outgoing message -->
<xsl:if test="@direction='0'">
<xsl:attribute name="bgcolor">#e5e5e5</xsl:attribute> <!-- outgoing -->
</xsl:if>
<xsl:if test="@direction='1'">
<xsl:attribute name="bgcolor">#b0b0b0</xsl:attribute> <!-- incoming -->
</xsl:if>

<tr>
<xsl:choose>

<!-- header with changed user state :: BEGIN -->
<xsl:when test="@direction='2'">
<!--dummy :); change of user state are single messages, becose of ??parser?? bug -->
</xsl:when>
<!-- header with changed user state :: END -->

<xsl:otherwise>

<!-- Icons :: BEGIN -->
<td nowrap="yes" width="60">

<!-- message link & message icon :: BEGIN -->
<a>
<xsl:attribute name="href">msg://<xsl:value-of select="id"/></xsl:attribute>
<img><xsl:attribute name="src">sim:icons/<xsl:value-of select="icon"/></xsl:attribute></img>
</a>
<xsl:text>&#xA0;</xsl:text>
<!-- message link & message icon :: END 
_____________________________________________________________________________-->
<!-- encrypting icon :: BEGIN -->
<xsl:if test="@encrypted='1'">
<img src="sim:icons/encrypted"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!-- encrypting icon :: END
_____________________________________________________________________________-->
<!--urgent message icon (DND etc) :: BEGIN -->
<xsl:if test="@urgent='1'">
<img src="sim:icons/urgentmsg"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!--urgent message icon (DND etc) :: END 
_____________________________________________________________________________-->
<!--listmessage (???) icon :: BEGIN -->
<xsl:if test="@list='1'">
<img src="sim:icons/listmsg"/>
<xsl:text>&#xA0;</xsl:text>
</xsl:if>
<!--listmessage (???) icon :: END -->
<!-- Icons :: END 
_____________________________________________________________________________-->

<!-- Sender name :: BEGIN -->
<span>

<!-- BOLD header if unread message -->
<xsl:if test="@unread='1'">
<xsl:attribute name="style">font-weight:600</xsl:attribute>
</xsl:if>

<xsl:text> </xsl:text>
<font>
<xsl:choose>
<xsl:when test="@direction='1'">
<xsl:attribute name="color">#800000</xsl:attribute>
</xsl:when>
<xsl:when test="@direction='0'">
<xsl:attribute name="color">#000080</xsl:attribute>
</xsl:when>
</xsl:choose>
<xsl:value-of disable-output-escaping="no" select="from"/>
</font>
<xsl:text> </xsl:text>
</span>
</td>
<!-- Sender name :: END 
_____________________________________________________________________________-->


<!-- Message time :: BEGIN -->
<td align="right" nowrap="yes">
<font>
<xsl:if test="@direction='2'">
<xsl:attribute name="color">#ffffff</xsl:attribute>
</xsl:if>
<xsl:text> </xsl:text>
<xsl:value-of select="time/date"/>
<xsl:text> </xsl:text>
<xsl:value-of select="time/hour"/>:<xsl:value-of select="time/minute"/>:<xsl:value-of select="time/second"/>
</font>
</td>
<!-- Message time :: END
_____________________________________________________________________________-->


</xsl:otherwise>
</xsl:choose>
</tr>
</table>
<!-- message header :: END
_____________________________________________________________________________-->



<xsl:choose>
<!-- message with changed user state :: BEGIN -->

<xsl:when test="@direction='2'">
<p>
<!-- message link & message icon :: BEGIN -->
<img><xsl:attribute name="src">sim:icons/<xsl:value-of select="icon"/></xsl:attribute></img>
<xsl:text>&#xA0;</xsl:text>
<!-- message link & message icon :: END
_____________________________________________________________________________-->
<font>
<!--xsl:attribute name="color">#ffffff</xsl:attribute-->
<xsl:text> </xsl:text>
<xsl:value-of select="time/date"/>
<xsl:text> </xsl:text>
<xsl:value-of select="time/hour"/>:<xsl:value-of select="time/minute"/>:<xsl:value-of select="time/second"/>
<xsl:text> </xsl:text>
<xsl:value-of disable-output-escaping="yes" select="from"/>
<xsl:text> --> </xsl:text><xsl:value-of disable-output-escaping="yes" select="body"/>
</font>
</p>
</xsl:when>
<!-- message with changed user state :: END
_____________________________________________________________________________-->

<!-- message body :: BEGIN -->
<xsl:otherwise>
<p>
<xsl:value-of disable-output-escaping="yes" select="body"/>
</p>
</xsl:otherwise>
</xsl:choose>
<!-- message body :: END
_____________________________________________________________________________-->


<!-- CRLF for formating -->
<p>
<xsl:text>&#xA0;</xsl:text>
</p>


</xsl:template>
</xsl:stylesheet>